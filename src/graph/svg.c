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
 * Filename		: svg.c
 * Description : svg
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 06/04/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_svg_c__
#define __libaroma_svg_c__
#include <aroma_internal.h>
#ifdef __cplusplus
extern "C" {
#endif
#ifndef LIBAROMA_CONFIG_NOSVG

/* INCLUDE SVG */
#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION
#include "svg.h"
#include "svg_wrapper.h"

LIBAROMA_CANVASP libaroma_svg_ex2(
		LIBAROMA_STREAMP stream,
		byte freeStream,
		byte use_px, 
		float dpi) {
	if (!stream) return NULL;
	char * data = libaroma_stream_to_string(stream,0);
	LIBAROMA_CANVASP ret = libaroma_svg_string(data, use_px, dpi);
	if (freeStream) {
		libaroma_stream_close(stream);
	}
	return ret;
}

LIBAROMA_CANVASP libaroma_svg_string(
		char *data,
		byte use_px, 
		float dpi) {

	if (!data) {
		return NULL;
	}

	LIBAROMA_CANVASP cv = NULL;
	NSVGimage *image = NULL;
	image=nsvgParse(data, use_px?"px":"dp", dpi);
	
	if (image == NULL) {
		ALOGW("libaroma_svg: Could not open SVG image.");
		goto exit;
	}
	ALOGV("libaroma_svg: Parsed SVG image (%dx%d)", (int)image->width, (int)image->height);

	NSVGrasterizer *rast =nsvgCreateRasterizer();
	if (rast == NULL) {
		ALOGW("libaroma_svg: Could not init rasterizer.");
		nsvgDelete(image);
		goto exit;
	}
	if (!use_px){
		cv = libaroma_canvas_ex(libaroma_dp(image->width),libaroma_dp(image->height),1);
	}
	else{
		cv = libaroma_canvas_ex(image->width,image->height,1);
	}
	if (cv == NULL){
		ALOGW("libaroma_svg: Could not create image canvas.");
		nsvgDelete(image);
		goto exit;
	}
	libaroma_canvas_setcolor(cv,0,0);
	nsvgRasterize(rast,image,0,0,1,cv);
	nsvgDelete(image);
	nsvgDeleteRasterizer(rast);

exit:
	return cv;
}

#endif /* LIBAROMA_CONFIG_NOSVG */
#ifdef __cplusplus
}
#endif
#endif /* __libaroma_svg_c__ */

