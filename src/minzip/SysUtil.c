/*
 * Copyright 2006 The Android Open Source Project
 *
 * System utilities.
 */
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#ifndef SIZE_MAX
# ifdef __SIZE_MAX__
#  define SIZE_MAX __SIZE_MAX__
# else
#  define SIZE_MAX std::numeric_limits<size_t>::max()
# endif
#endif
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <sys/mman.h>
#endif /*_WIN32*/
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define LOG_TAG "sysutil"
#include "Log.h"
#include "SysUtil.h"

#ifdef __cplusplus
extern "C" {
#endif

static int sysMapBlockFile(FILE* mapf, MemMapping* pMap)
{
#ifdef _WIN32
   LOGE("Map of blocks aren't supported under Win32\n");
   return -1;
#else
    char block_dev[PATH_MAX+1];
    size_t size;
    unsigned int blksize;
    size_t blocks;
    unsigned int range_count;
    unsigned int i;

    if (fgets(block_dev, sizeof(block_dev), mapf) == NULL) {
        LOGE("failed to read block device from header\n");
        return -1;
    }
    for (i = 0; i < sizeof(block_dev); ++i) {
        if (block_dev[i] == '\n') {
            block_dev[i] = 0;
            break;
        }
    }

    if (fscanf(mapf, "%zu %u\n%u\n", &size, &blksize, &range_count) != 3) {
        LOGE("failed to parse block map header\n");
        return -1;
    }
    if (blksize != 0) {
        blocks = ((size-1) / blksize) + 1;
    }
    if (size == 0 || blksize == 0 || blocks > SIZE_MAX / blksize || range_count == 0) {
        LOGE("invalid data in block map file: size %zu, blksize %u, range_count %u\n",
             size, blksize, range_count);
        return -1;
    }

    pMap->range_count = range_count;
    pMap->ranges = calloc(range_count, sizeof(MappedRange));
    if (pMap->ranges == NULL) {
        LOGE("calloc(%u, %zu) failed: %s\n", range_count, sizeof(MappedRange), strerror(errno));
        return -1;
    }

    // Reserve enough contiguous address space for the whole file.
    unsigned char* reserve;	
    reserve = mmap(NULL, blocks * blksize, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);

    if (reserve == MAP_FAILED) {
        LOGE("failed to reserve address space: %s\n", strerror(errno));
        free(pMap->ranges);
        return -1;
    }

    int fd = open(block_dev, O_RDONLY);
    if (fd < 0) {
        LOGE("failed to open block device %s: %s\n", block_dev, strerror(errno));
        munmap(reserve, blocks * blksize);
        free(pMap->ranges);
        return -1;
    }

    unsigned char* next = reserve;
    size_t remaining_size = blocks * blksize;
    bool success = true;
    for (i = 0; i < range_count; ++i) {
        size_t start, end;
        if (fscanf(mapf, "%zu %zu\n", &start, &end) != 2) {
            LOGE("failed to parse range %d in block map\n", i);
            success = false;
            break;
        }
        size_t length = (end - start) * blksize;
        if (end <= start || (end - start) > SIZE_MAX / blksize || length > remaining_size) {
          LOGE("unexpected range in block map: %zu %zu\n", start, end);
          success = false;
          break;
        }
		/* initialization makes pointer from integer without a cast */
        void* addr = mmap(next, length, PROT_READ, MAP_PRIVATE | MAP_FIXED, fd, ((off64_t)start)*blksize);
        if (addr == MAP_FAILED) {
            LOGE("failed to map block %d: %s\n", i, strerror(errno));
            success = false;
            break;
        }
        pMap->ranges[i].addr = addr;
        pMap->ranges[i].length = length;

        next += length;
        remaining_size -= length;
    }
    if (success && remaining_size != 0) {
      LOGE("ranges in block map are invalid: remaining_size = %zu\n", remaining_size);
      success = false;
    }
    if (!success) {
      close(fd);
      munmap(reserve, blocks * blksize);
      free(pMap->ranges);
      return -1;
    }

    close(fd);
    pMap->addr = reserve;
    pMap->length = size;

    LOGI("mmapped %d ranges\n", range_count);

    return 0;
#endif /* _WIN32 */
}

int sysMapFile(char* fn, MemMapping* pMap)
{
	if (fn==NULL) return -1;
    memset(pMap, 0, sizeof(*pMap));
	
    if (fn[0] == '@') {
    // A map of blocks
        FILE* mapf = fopen(fn+1, "r");
        if (mapf == NULL) {
            LOGE("Unable to open '%s': %s\n", fn+1, strerror(errno));
            return -1;
        }
        if (sysMapBlockFile(mapf, pMap) != 0) {
            LOGE("Map of '%s' failed\n", fn);
            fclose(mapf);
            return -1;
        }
        fclose(mapf);
    }
	else {
		// This is a regular file.
		LIBAROMA_STREAMP file = libaroma_stream_file(fn);
		if (file == NULL) {
			LOGE("failed to load file %s\n", fn);
			return -1;
		}

		pMap->addr = file->data;
		pMap->length = file->size;
		pMap->stream = file;
		pMap->range_count = 1;
		pMap->ranges = malloc(sizeof(MappedRange));
		if (pMap->ranges == NULL) {
			LOGE("malloc failed: %s\n", strerror(errno));
			libaroma_stream_close(file);
			return -1;
		}
		pMap->ranges[0].addr = file->data;
		pMap->ranges[0].length = file->size;
	}
    return 0;
}

/*
 * Release a memory mapping.
 */
void sysReleaseMap(MemMapping* pMap)
{
	if (pMap->stream!=NULL){
		libaroma_stream_close(pMap->stream);
	}
	#ifndef _WIN32
	else {
		int i;
		for (i = 0; i < pMap->range_count; ++i) {
			if (munmap(pMap->ranges[i].addr, pMap->ranges[i].length) < 0) {
				LOGE("munmap(%p, %d) failed: %s\n",
					 pMap->ranges[i].addr, (int)pMap->ranges[i].length, strerror(errno));
			}
		}
	}
	#endif /* _WIN32 */
    free(pMap->ranges);
    pMap->ranges = NULL;
    pMap->range_count = 0;
}

#ifdef __cplusplus
}
#endif