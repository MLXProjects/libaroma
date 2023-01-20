/********************************************************************[libaroma]*
 * Copyright (C) 2011-2023 Ahmad Amarullah (http://amarullz.com/)
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
 * Filename		: fb_drm.h
 * Description : drm fb header
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 26/02/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_linux_fb_drm_driver_h__
#define __libaroma_linux_fb_drm_driver_h__

#ifndef LIBAROMA_CONFIG_NODRM
#include <drm_fourcc.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#ifdef __cplusplus
extern "C" {
#endif

/* drm internal data */
typedef struct{
	/* the first variable must be in any backend_internal struct */
	byte type;
	
	/* buffer in use */
	int current_buffer;
	
	/* resources in use */
	drmModeRes *res;
	drmModeCrtc *crtc;
	drmModeConnector *con;
	
	/* buffers information */
	void *buffer[2];
	unsigned int buffer_id[2];
	unsigned int buffer_sz[2];
	unsigned int buffer_stride[2];
} DRMFB_INTERNAL, * DRMFB_INTERNALP;


/*
 * Function		: DRMFB_init
 * Return Value: byte
 * Descriptions: initialize dri device
 */
extern byte DRMFB_init(LIBAROMA_FBP me);

/*
 * Function		: DRMFB_end_post
 * Return Value: byte
 * Descriptions: start post
 */
extern byte DRMFB_start_post(LIBAROMA_FBP me);

/*
 * Function		: DRMFB_post
 * Return Value: byte
 * Descriptions: post
 */
extern byte DRMFB_post(
	LIBAROMA_FBP me, wordp __restrict src,
	int dx, int dy, int dw, int dh,
	int sx, int sy, int sw, int sh
	);

/*
 * Function		: DRMFB_end_post
 * Return Value: byte
 * Descriptions: end post
 */
extern byte DRMFB_end_post(LIBAROMA_FBP me);

/*
 * Function		: DRMFB_flush
 * Return Value: void
 * Descriptions: flush display update
 */
extern byte DRMFB_flush(LIBAROMA_FBP me);

/*
 * Function		: DRMFB_release
 * Return Value: void
 * Descriptions: release drm driver
 */
extern void DRMFB_release(LIBAROMA_FBP me); 

#ifdef __cplusplus
}
#endif

#endif /* LIBAROMA_CONFIG_NODRM */
#endif /* __libaroma_linux_fb_qcom_driver_h__ */
