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
 * Filename		: image.c
 * Description : image
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 06/04/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_image_c__
#define __libaroma_image_c__
#include <aroma_internal.h>
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Function		: libaroma_image_ex
 * Return Value: LIBAROMA_CANVASP
 * Descriptions: read image by file signature - extended
 */
LIBAROMA_CANVASP libaroma_image_ex(
		LIBAROMA_STREAMP stream,
		byte freeStream,
		byte hicolor){
	if (!stream) {
		return NULL;
	}
	if (stream->size<5){
		goto errorgo;
	}
	bytep d;
	d=stream->data;

	/* png */
	if (
		(d[0]==0x89)&&
		(d[1]==0x50)&&
		(d[2]==0x4E)&&
		(d[3]==0x47)){
		return libaroma_png_ex(stream,freeStream,hicolor);
	}
#ifndef LIBAROMA_CONFIG_NOJPEG
	/* jpeg */
	if (
		(d[0]==0xFF)&&
		(d[1]==0xD8)&&
		(d[2]==0xFF)){
		return libaroma_jpeg_ex(stream,freeStream,hicolor);
	}
#endif

#ifndef LIBAROMA_CONFIG_NOSVG
	/* svg */
	byte is_svg;
	is_svg=0;
	if (libaroma_stristr(stream->uri, ".svg", stream->size)!=NULL){
		is_svg=1;
	}
	else{
		int i;
		for (i=0;((i<stream->size)&&(i<2048));i++){
			switch (d[i]){
				case '<':
					is_svg=1;
					i=10000;
					break;
				case ' ':
				case '\n':
				case '\r':
				case '\t':
					break;
				default:
					i=10000;
			}
		}
	}
	if (is_svg){
		return libaroma_svg_ex(stream,freeStream,0);
	}
#endif

errorgo:
	ALOGW("libaroma_image_new \"%s\" not valid image", stream->uri);
	if (freeStream){
		if (stream){
			libaroma_stream_close(stream);
		}
	}
	return NULL;
} /* End of libaroma_image_new */

/*
 * Function		: libaroma_image_rotate90
 * Return Value: void
 * Descriptions: copy & rotate image data (90 degrees)
 */
byte libaroma_image_rotate90(
		bytep dst, bytep src,
		int dx, int dy, int dw, int dh, int d_maxw, int d_linesz, int d_pixsz,
		int sx, int sy, int s_linesz, int s_pixsz){
	/* set destination & source to offset x/y */
	dst += (d_linesz*dx) + ((d_maxw-dy)*d_pixsz) - d_pixsz;
	src += (s_linesz*sy) + (sx*s_pixsz);
	/* xmax = dest width * source pixel size */
	int x, y, xmax=dw*s_pixsz;
	/* if dest & src pixel size are the same, just use normal copy */
	if (d_pixsz == s_pixsz){
		/* loop through source columns */
		for (y=0; y<dh; y++){
		#ifdef LIBAROMA_CONFIG_OPENMP
			#pragma omp parallel for
		#endif
			/* loop through current source row */
			for (x=0; x<xmax; x+=s_pixsz){
				/* copy 1 pixel each iteration until end of row */
				memcpy(dst+(d_maxw*x), src+x, s_pixsz);
			}
			/* move dest offset one pixel at the left */
			dst -= d_pixsz;
			/* move source offset one line below */
			src += s_linesz;
		}
	}
	/* TODO: convert 32 to 16 bit and vice versa
	else if (d_pixsz == 4 && s_pixsz == 2){
	}*/
	else {
		ALOGW("image_rotate90 pixel conversion not supported");
		return 0;
	}
	return 1;
} /* End of libaroma_image_rotate90 */

/*
 * Function		: libaroma_image_rotate180
 * Return Value: void
 * Descriptions: copy & rotate image data (180 degrees)
 */
byte libaroma_image_rotate180(
		bytep dst, bytep src,
		int dx, int dy, int dw, int dh, int d_maxh, int d_linesz, int d_pixsz,
		int sx, int sy, int s_linesz, int s_pixsz){
	/* set destination to last pixel offset */
	dst +=  (d_linesz*(d_maxh-dy)) - (dx*d_pixsz) - d_pixsz;
	/* set source to offset x/y */
	src += (s_linesz*sy) + (sx*s_pixsz);
	/* xmax = destination width in bytes */
	int x, y, xmax = dw*d_pixsz;
	/* if dest & src pixel size are the same, just use normal copy */
	if (d_pixsz == s_pixsz){
	/* loop through source/dest columns */
		for (y=0; y<dh; y++){
		#ifdef LIBAROMA_CONFIG_OPENMP
			#pragma omp parallel for
		#endif
			/* loop through horizontal pixels in current row */
			for (x = 0; x < xmax; x+=s_pixsz){
				/* copy 1 pixel each iteration until end of row */
				memcpy(dst-x, src+x, s_pixsz);
			}
			/* move dest offset one line above */
			dst -= d_linesz;
			/* move source offset one line below */
			src += s_linesz;
		}
	}
	/* TODO: convert 32 to 16 bit and vice versa
	else if (d_pixsz == 4 && s_pixsz == 2){
	}*/
	else {
		ALOGW("image_rotate180 pixel conversion not supported");
		return 0;
	}
	return 1;
} /* End of libaroma_image_rotate180 */

/*
 * Function		: libaroma_image_rotate270
 * Return Value: void
 * Descriptions: copy & rotate image data (270 degrees)
 */
byte libaroma_image_rotate270(
		bytep dst, bytep src,
		int dx, int dy, int dw, int dh, int d_maxh, int d_linesz, int d_pixsz,
		int sx, int sy, int s_linesz, int s_pixsz){
	/* set destination & source to offset x/y */
	dst += (d_linesz*(d_maxh-dx)) + (dy*d_pixsz) - d_linesz;
	src += (s_linesz*sy) + (sx*s_pixsz);
	int x, y;
	/* if dest & src pixel size are the same, just use normal copy */
	if (d_pixsz == s_pixsz){
		/* loop through source columns */
		for (y=0; y<dh; y++){
		#ifdef LIBAROMA_CONFIG_OPENMP
			#pragma omp parallel for
		#endif
			/* loop through horizontal pixels in current source row */
			for (x=0; x<dw; x++){
				/* copy 1 pixel each iteration until end of row */
				memcpy(dst-(d_linesz*x)+s_pixsz, src+(x*s_pixsz), s_pixsz);
			}
			/* move dest offset one pixel at the right */
			dst += d_pixsz;
			/* move source offset one line below */
			src += s_linesz;
		}
	}
	/* TODO: convert 32 to 16 bit and vice versa
	else if (d_pixsz == 4 && s_pixsz == 2){
	}*/
	else {
		ALOGW("image_rotate270 pixel conversion not supported");
		return 0;
	}
	return 1;
} /* End of libaroma_image_rotate270 */
#ifdef __cplusplus
}
#endif

#endif /* __libaroma_image_c__ */

