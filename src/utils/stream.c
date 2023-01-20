/********************************************************************[libaroma]*
 * Copyright (C) 2011-2015 Ahmad Amarullah (http://amarullz.com/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *			http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *______________________________________________________________________________
 *
 * Filename		: stream.c
 * Description : stream utility
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 19/01/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_stream_c__
#define __libaroma_stream_c__
#include <aroma_internal.h>
#include <sys/stat.h>

#define LIBAROMA_STREAMDATA_CHUNK 4096

#ifdef __cplusplus
extern "C" {
#endif
/*
 * Variable		: _libaroma_stream_uri_cb
 * Type				: LIBAROMA_STREAM_URI_CB
 * Descriptions: stream uri callback storage
 */
static LIBAROMA_STREAM_URI_CB _libaroma_stream_uri_cb = NULL;

/*
 * Function		: libaroma_stream_set_uri_callback
 * Return Value: void
 * Descriptions: set stream uri callback
 */
void libaroma_stream_set_uri_callback(
		LIBAROMA_STREAM_URI_CB cb) {
	_libaroma_stream_uri_cb = cb;
} /* End of libaroma_stream_set_uri_callback */

/*
 * Function		: libaroma_stream_file
 * Return Value: LIBAROMA_STREAMP
 * Descriptions: new stream from file - extended
 */
LIBAROMA_STREAMP libaroma_stream_file_ex(
		char * path, byte tostring) {
	LIBAROMA_STREAMP ret;
#ifdef LIBAROMA_PLATFORM_HAS_MMAP
	if (!path) {
		ALOGW("libaroma_stream_file path is invalid");
		return NULL;
	}
	/* Read File Stat */
	struct stat st;
	int filesize=stat(path, &st);
	if (filesize < 0) {
		ALOGI("libaroma_stream_file (%s) not found", path);
		return NULL;
	}
	/* Open File */
	int fd = open(path, O_RDONLY, 0);
	if (fd < 0) {
		ALOGW("libaroma_stream_file unable to open (%s)", path);
		return NULL;
	}
	/* MAP */
	bytep mem = (bytep) mmap(NULL, filesize, PROT_READ, MAP_FILE | MAP_SHARED, fd, 0);
	/* Close FD */
	close(fd);
	if (mem == MAP_FAILED) {
		ALOGW("libaroma_stream_file unable to mmap (%s)", path);
		/* retry without mmap */
		goto nommap;
	}
	/* Return */
	ret = (LIBAROMA_STREAMP) malloc(sizeof(LIBAROMA_STREAM));
	if (tostring){
		ret->size	= filesize + 1;
		ret->data	= malloc(ret->size);
		memcpy(ret->data, mem, ret->size);
		ret->data[ret->size] = '\0';
		ret->ismmap=1;
		ret->isalloc =1; //malloc'ed data
	}
	else {
		ret->data	= mem;
		ret->size	= filesize;
		ret->ismmap	= 1;
	}
	snprintf(ret->uri,
			LIBAROMA_STREAM_URI_LENGTH, "file://%s", path);
#endif
nommap:
	if (!path) {
		ALOGW("libaroma_stream_file path is invalid");
		return 0;
	}
	/* open file handle */
	FILE *in = fopen(path, "rb");
	if (in==NULL) {
		return NULL;
	}
    bytep data = NULL, temp;
    unsigned int size = 0;
    size_t used = 0;
    size_t n;

    /* A read error already occurred? */
    if (ferror(in)){
        ALOGW("libaroma_stream_file initial read error");
        return NULL;
	}
    while (1) {
        if (used + LIBAROMA_STREAMDATA_CHUNK + (tostring?1:0) > size) {
            size = used + LIBAROMA_STREAMDATA_CHUNK + (tostring?1:0);
            /* Overflow check. Some ANSI C compilers
               may optimize this away, though. */
            if (size <= used) {
                ALOGW("libaroma_stream_file too much data read");
                goto error;
            }
            temp = realloc(data, size);
            if (temp == NULL) {
				ALOGW("libaroma_stream_file not enough memory");
				goto error;
            }
            data = temp;
        }
        n = fread(data + used, 1, LIBAROMA_STREAMDATA_CHUNK, in);
        if (n == 0)
            break;
        used += n;
    }
    if (ferror(in)) {
		ALOGW("libaroma_stream_file read error");
		goto error;
    }
	/* close file handle */
	fclose(in);
	if (tostring){
		temp = realloc(data, used + 1);
		if (temp == NULL) {
			ALOGW("libaroma_stream_file not enough memory");
			goto error;
		}
		temp[used]='\0';
		data = temp;
	}
	goto done;
error:
	free(data);
	return NULL;
done:
	ret = (LIBAROMA_STREAMP) malloc(sizeof(LIBAROMA_STREAM));
	ret->data	= data;
	ret->size	= used;
	ret->ismmap	= 0;
	ret->isalloc= 1; /* if not using mmap, data is always malloc'ed */
	snprintf(ret->uri,
			LIBAROMA_STREAM_URI_LENGTH, "file://%s", path);
	return ret;
} /* End of libaroma_stream_file */

/*
 * Function		: libaroma_stream_shmem
 * Return Value: LIBAROMA_STREAMP
 * Descriptions: new stream from shmem
 */
LIBAROMA_STREAMP libaroma_stream_shmem(
		char * memname) {
#ifdef LIBAROMA_PLATFORM_HAS_SHMEM
	if (!memname) {
		ALOGW("libaroma_stream_shmem memname is invalid");
		return 0;
	}
	/* set name */
	char nm[LIBAROMA_STREAM_URI_LENGTH];
	if (memname[0]=='@'){
		snprintf(nm, LIBAROMA_STREAM_URI_LENGTH,
			"%s%s", LIBAROMA_CONFIG_SHMEM_PREFIX, memname+1);
	}
	else{
		snprintf(nm, LIBAROMA_STREAM_URI_LENGTH, "%s", memname);
	}

	/* open */
	int fd = shm_open(nm, O_RDWR, 0666);
	if (fd < 0) {
		ALOGW("libaroma_stream_shmem shm_open failed (%s)",memname);
		return 0;
	}

	/* read shmem stat */
	struct stat st;
	int filesize=stat(nm, &st);
	if (filesize < 0) {
		ALOGW("libaroma_stream_shmem stat is invalid (%s)", nm);
		close(fd);
		return 0;
	}

	/* MAP */
	bytep mem = (bytep) mmap(NULL, filesize, PROT_READ, MAP_SHARED, fd, 0);
	/* close fd */
	close(fd);

	if (mem == MAP_FAILED) {
		ALOGW("libaroma_stream_shmem unable to mmap (%s)", nm);
		return 0;
	}
	/* Return */
	LIBAROMA_STREAMP ret = (LIBAROMA_STREAMP) malloc(sizeof(LIBAROMA_STREAM));
	ret->data	= mem;
	ret->size	= filesize;
	ret->ismmap	= 1;
	ret->isalloc= 0;
	snprintf(ret->uri,
			LIBAROMA_STREAM_URI_LENGTH, "shmem://%s", memname);
	return ret;
#else
	return NULL;
#endif
} /* End of libaroma_stream_shmem */

/*
 * Function		: libaroma_stream_mzip
 * Return Value: LIBAROMA_STREAMP
 * Descriptions: new stream from opened zip
 */
LIBAROMA_STREAMP libaroma_stream_mzip(
		LIBAROMA_ZIP zip,
		char * zpath) {
#ifndef LIBAROMA_CONFIG_NOMINZIP
	if ((!zip) || (!zpath)) {
		ALOGW("libaroma_stream_hzip zip/zpath is invalid");
		return 0;
	}
	/* Read From Zip */
	bytep mem;
	int mem_sz = libaroma_zip_read(zip, &mem, zpath, 1);
	/* It's not valid */
	if (mem_sz == 0) {
		return 0;
	}
	LIBAROMA_STREAMP ret	= (LIBAROMA_STREAMP) malloc(sizeof(LIBAROMA_STREAM));
	ret->data		 = mem;
	ret->size		 = mem_sz;
	ret->ismmap	 = 0;
	ret->isalloc		= 0;
	snprintf(ret->uri, LIBAROMA_STREAM_URI_LENGTH,
			"memzip://[zip_resource]#%s", zpath);
	return ret;
#else
	return NULL;
#endif
} /* End of libaroma_stream_mzip */

/*
 * Function		: libaroma_stream_zip
 * Return Value: LIBAROMA_STREAMP
 * Descriptions: new stream from zip file
 */
LIBAROMA_STREAMP libaroma_stream_zip(
		char * zip_path, char * zpath) {
#ifndef LIBAROMA_CONFIG_NOMINZIP
	if ((!zip_path) || (!zpath)) {
		ALOGW("libaroma_stream_zip zip_path/zpath is invalid");
		return 0;
	}
	/* Init Zip */
	LIBAROMA_ZIP zip = libaroma_zip(zip_path);
	if (!zip) {
		return 0;
	}
	/* Allocating Adapter */
	bytep mem;
	int mem_sz = libaroma_zip_read(zip, &mem, zpath, 1);
	/* Release ZIP */
	libaroma_zip_release(zip);
	/* not valid */
	if (mem_sz == 0) {
		return NULL;
	}
	LIBAROMA_STREAMP ret = (LIBAROMA_STREAMP) malloc(sizeof(LIBAROMA_STREAM));
	ret->data	= mem;
	ret->size	= mem_sz;
	ret->ismmap	= 0;
	snprintf(ret->uri, LIBAROMA_STREAM_URI_LENGTH,
			"zip://%s#%s", zip_path, zpath);
	return ret;
#else
	return NULL;
#endif
} /* End of libaroma_stream_zip */

/*
 * Function		: libaroma_stream_mem
 * Return Value: LIBAROMA_STREAMP
 * Descriptions: new stream from memory range
 */
LIBAROMA_STREAMP libaroma_stream_mem(
		bytep mem, int mem_sz) {
	if (!mem || !mem_sz) {
		ALOGW("libaroma_stream_mem mem/mem_sz is invalid");
		return 0;
	}
	LIBAROMA_STREAMP ret = (LIBAROMA_STREAMP) malloc(sizeof(LIBAROMA_STREAM));
	ret->data	= mem;
	ret->size	= mem_sz;
	ret->ismmap	= 0;
	ret->isalloc	= 1;
	snprintf(ret->uri, LIBAROMA_STREAM_URI_LENGTH,
			"mem://0x%x-0x%x", *((unsigned int*)mem), mem_sz);
	return ret;
} /* End of libaroma_stream_mem */

/*
 * Function		: libaroma_stream
 * Return Value: LIBAROMA_STREAMP
 * Descriptions: new stream from uri
 */
LIBAROMA_STREAMP libaroma_stream(
		char * uri) {
	int n = strlen(uri);
	char kwd[11];
	int i;
	for (i = 0; i < n && i < 10; i++) {
		kwd[i] = uri[i];
		kwd[i + 1] = 0;
		if ((i > 1) && (uri[i] == '/') && (uri[i - 1] == '/')) {
			break;
		}
	}
	if (strcmp(kwd, "file://") == 0) {
		return libaroma_stream_file(uri + 7);
	}
	else if (strcmp(kwd, "shmem://") == 0) {
		return libaroma_stream_shmem(uri + 8);
	}
#ifndef LIBAROMA_CONFIG_NOMINZIP
	else if (strcmp(kwd, "zip://") == 0) {
		char zip_path[256] = {0};
		char zpath[256] = {0};
		int	cpos = 0;
		byte is_zpath = 0;
		for (i = 6; i < n && i < 255; i++) {
			if (is_zpath) {
				zpath[cpos++] = uri[i];
				zpath[cpos] = 0;
			}
			else {
				if (uri[i] == '#') {
					cpos		 = 0;
					is_zpath = 1;
				}
				else {
					zip_path[cpos++] = uri[i];
					zip_path[cpos] = 0;
				}
			}
		}
		return libaroma_stream_zip(zip_path, zpath);
	}
#endif
	else if (strcmp(kwd, "mem://") == 0) {
		unsigned int mem;
		int mem_sz;
		int rc;

		rc = sscanf(uri+6, "0x%x-0x%x", &mem, &mem_sz);
		if (rc!=2) {
			ALOGW("libaroma_stream_mem range format is invalid");
			return NULL;
		}

		return libaroma_stream_mem((bytep)&mem, mem_sz);
	}
	else if (_libaroma_stream_uri_cb != NULL) {
		return _libaroma_stream_uri_cb(uri);
	}
	return NULL;
} /* End of libaroma_stream */

/*
 * Function		: libaroma_stream_close
 * Return Value: byte
 * Descriptions: close stream
 */
byte libaroma_stream_close(
		LIBAROMA_STREAMP a) {
	if (!a) {
		ALOGW("libaroma_stream_close stream is invalid");
		return 0;
	}
#ifdef LIBAROMA_PLATFORM_HAS_MMAP
	if (a->ismmap) {
		/* File */
		munmap(a->data, a->size);
	}
#endif
	if (a->isalloc == 1 && !a->ismmap){
		free(a->data);
		a->data=NULL;
	}
	free(a);
	return 1;
} /* End of libaroma_stream_close */

/*
 * Function		: libaroma_stream_to_string
 * Return Value: char *
 * Descriptions: convert stream to string
 */
char * libaroma_stream_to_string(
		LIBAROMA_STREAMP a,
		byte free) {
	if (!a) {
		return NULL;
	}
	char * txt = malloc(a->size + 1);
	memcpy(txt, a->data, a->size);
	txt[a->size] = 0;
	if (free) {
		libaroma_stream_close(a);
	}
	return txt;
} /* End of libaroma_stream_to_string */

/*
 * Function		: libaroma_shmem
 * Return Value: LIBAROMA_SHMEMP
 * Descriptions: new/open shared memory
 */
LIBAROMA_SHMEMP libaroma_shmem_ex(
		const char * name,
		int sz, byte truncate) {
#ifdef LIBAROMA_PLATFORM_HAS_SHMEM
	/* Copy Name */
	char nm[LIBAROMA_STREAM_URI_LENGTH];
	if (name[0]=='@'){
		snprintf(nm, LIBAROMA_STREAM_URI_LENGTH,
			"%s%s", LIBAROMA_CONFIG_SHMEM_PREFIX, name+1);
	}
	else{
		snprintf(nm, LIBAROMA_STREAM_URI_LENGTH,
			"%s", name);
	}

	int fd;
	if (sz < 1) {
		/* Open Shared Memory */
		fd = shm_open(nm, O_RDWR, 0666);
		if (fd < 0) {
			ALOGW("libaroma_shmem shm_open failed (%s)", nm);
			return NULL;
		}
		struct stat st;
		int open_sz = fstat(fd, &st);
		if (open_sz < 1) {
			ALOGW("libaroma_shmem exiting invalid size (%s)", nm);
			close(fd);
			return NULL;
		}
		ALOGV("libaroma_shmem reopen shmem - %ibytes (%s)", sz, nm);
	}
	else {
		/* Create Shared Memory */
		fd = shm_open(nm, O_CREAT | O_TRUNC | O_RDWR, 0666);
		if (fd < 0) {
			ALOGW("libaroma_shmem shm_open failed (%s)", nm);
			return NULL;
		}
		/* Check shmem size */
		struct stat st;
		int open_sz = fstat(fd, &st);
		/* Truncate */
		/* if truncation disabled, throw error */
		if (!truncate && open_sz != sz) {
			ALOGW("libaroma_shmem exiting different size (%s)", nm);
			close(fd);
			return NULL;
		}
		if (ftruncate(fd, sz) != 0) {
			ALOGW("libaroma_shmem ftruncate error (%s)", nm);
			close(fd);
			return NULL;
		}
		ALOGV("libaroma_shmem new - %ibytes (%s)", sz, nm);
	}
	/* mmap */
	bytep mem = (bytep) mmap(0, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	/* check memory */
	if (mem == MAP_FAILED) {
		ALOGW("libaroma_shmem unable to mmap (%s)", name);
		return NULL;
	}
	/* return data */
	LIBAROMA_SHMEMP ret	 = (LIBAROMA_SHMEMP) malloc(sizeof(LIBAROMA_SHMEM));
	ret->data		 = mem;
	ret->size		 = sz;
	snprintf(ret->name, LIBAROMA_STREAM_URI_LENGTH, "%s", nm);
	return ret;
#else
	return NULL;
#endif
} /* End of libaroma_shmem */

/*
 * Function		: libaroma_shmem_close
 * Return Value: byte
 * Descriptions: close/delete shared memory
 */
byte libaroma_shmem_close(
		LIBAROMA_SHMEMP a,
		byte del) {
#ifdef LIBAROMA_PLATFORM_HAS_SHMEM
	if (!a) {
		ALOGW("libaroma_shmem_close LIBAROMA_SHMEMP not valid");
		return 0;
	}
	if (a->data) {
		munmap(a->data, a->size);
	}
	if (del) {
		if (shm_unlink(a->name) != 0) {
			ALOGW("libaroma_shmem_close shm_unlink failed (%s)", a->name);
		}
	}
	free(a);
	return 1;
#else
	return 0;
#endif
} /* End of libaroma_shmem_close */

#ifdef __cplusplus
}
#endif
#endif /* __libaroma_stream_c__ */
