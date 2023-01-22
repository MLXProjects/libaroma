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
 * Filename		: zip.c
 * Description : zip utility
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 19/01/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_zip_c__
#define __libaroma_zip_c__
#include <aroma_internal.h>
#ifndef LIBAROMA_CONFIG_NOMINZIP
#include <zlib.h>
#include <Zip.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
/*
 * Function		: libaroma_zip
 * Return Value: LIBAROMA_ZIP (void*)
 * Descriptions: open zip file
 */
LIBAROMA_ZIP libaroma_zip(
		char * filename) {
	LIBAROMA_ZIP zip = NULL;
#ifndef LIBAROMA_CONFIG_NOMINZIP
	zip = (LIBAROMA_ZIP) malloc(sizeof(ZipArchive));
	if (zip == NULL){
		ALOGW("libaroma_zip cannot allocate memory");
		return NULL;
	}
	MemMapping map;
    if (sysMapFile(filename, &map) != 0) {
        printf("libaroma_zip failed to map package %s\n", filename);
        return NULL;
    }
	if (mzOpenZipArchive(map.addr, map.length, (ZipArchive *) zip) != 0) {
		ALOGW("libaroma_zip mzOpenZipArchive error (%s)", filename);
		free(zip);
		return NULL;
	}
#endif /* LIBAROMA_CONFIG_NOMINZIP */
	return zip;
} /* End of libaroma_zip */

/*
 * Function		: libaroma_zip_release
 * Return Value: void
 * Descriptions: release zip file
 */
void libaroma_zip_release(
		LIBAROMA_ZIP zip) {
#ifndef LIBAROMA_CONFIG_NOMINZIP
	mzCloseZipArchive((ZipArchive *) zip);
	free(zip);
#endif
} /* End of libaroma_zip_release */

/*
 * Function		: libaroma_zip_extract
 * Return Value: byte
 * Descriptions: extract zip entry to file
 */
byte libaroma_zip_extract(
		LIBAROMA_ZIP zip,
		char * zpath,
		char * dest) {
#if !defined(LIBAROMA_CONFIG_NOMINZIP) && defined(LIBAROMA_PLATFORM_HAS_FD)
	const ZipEntry * zdata = mzFindZipEntry((ZipArchive *) zip, zpath);
	if (zdata == NULL) {
		ALOGW("libaroma_zip_extract zdata=NULL (%s)", zpath);
		return 0;
	}
	libaroma_unlink(dest);
	int fd = creat(dest, 0755);
	if (fd < 0) {
		ALOGW("libaroma_zip_extract creat error (%s)", dest);
		return 0;
	}
	byte ok = mzExtractZipEntryToFile((ZipArchive *) zip, zdata, fd);
	close(fd);
	return ok;
#else
	return 0;
#endif
} /* End of libaroma_zip_extract */

/*
 * Function		: libaroma_zip_read
 * Return Value: int
 * Descriptions: read zip entry contents
 */
int libaroma_zip_read(
		LIBAROMA_ZIP zip,
		bytep * bufp,
		char * zpath,
		byte bytesafe) {
#ifndef LIBAROMA_CONFIG_NOMINZIP
	const ZipEntry * se = mzFindZipEntry((ZipArchive *) zip, zpath);
	if (se == NULL) {
		ALOGW("libaroma_zip_read mzFindZipEntry error (%s)", zpath);
		return 0;
	}
	int sz = se->uncompLen + (bytesafe ? 0 : 1);
	bytep buf = malloc(sz);
	if (!mzReadZipEntry((ZipArchive *) zip, se, (char *) buf, se->uncompLen)) {
		ALOGW("libaroma_zip_read mzReadZipEntry error (%s)", zpath);
		free(buf);
		return 0;
	}
	if (!bytesafe) {
		buf[se->uncompLen] = '\0';
	}
	*bufp = buf;
	return sz;
#else
	return 0;
#endif /* LIBAROMA_CONFIG_NOMINZIP */
} /* End of libaroma_zip_read */

#ifdef __cplusplus
}
#endif

#endif /* __libaroma_zip_c__ */
