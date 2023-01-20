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
 * Filename		: fb_drm.c
 * Description : linux framebuffer driver for drm/kms
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 17/01/23 - Author(s): Michael Jauregui
 *
 */
#ifndef __libaroma_linux_fb_drm_driver_c__
#define __libaroma_linux_fb_drm_driver_c__

#ifndef LIBAROMA_CONFIG_NODRM
#include <aroma_internal.h>
#include "fb.h"

#ifndef LIBAROMA_DRMFB_CARDNUM
#define LIBAROMA_DRMFB_CARDNUM "0"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* function prototypes */
static int DRMFB_mkdumb(LINUXFBDR_INTERNALP mi, int w, int h, int i);
static void DRMFB_deldumb(LINUXFBDR_INTERNALP mi, int i);
static void DRMFB_dump(LINUXFBDR_INTERNALP mi, DRMFB_INTERNALP di);

/*
 * Function		: DRMFB_init
 * Return Value: byte
 * Descriptions: initialize dri device
 */
byte DRMFB_init(LIBAROMA_FBP me){
	if (me == NULL) {
		return 0;
	}
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
	
	/* allocating drm internal data */
	mi->backend = calloc(sizeof(QCOMFB_INTERNAL),1);
	if (!mi->backend){
		ALOGW("DRMFB_init cannot allocate drm internal data");
		return 0;
	}
	DRMFB_INTERNALP di = (DRMFB_INTERNALP) mi->backend;
	
	/* set backend type */
	di->type = LINUXFBDR_BACKEND_DRM;
	
	/* variables for later use */
	int i, j, retval;
	drmModeEncoder *enc = NULL;
	/* open dri card */
	ALOGV("DRMFB_init open /dev/dri/card" LIBAROMA_DRMFB_CARDNUM);
	mi->fb = open("/dev/dri/card" LIBAROMA_DRMFB_CARDNUM, O_RDWR);
	if (mi->fb < 0){
		ALOGW("DRMFB_init failed to open /dev/dri/card" LIBAROMA_DRMFB_CARDNUM);
		free(di);
		mi->backend = NULL;
		return 0;
	}
	ALOGV("DRMFB_init Check for dumb buffers support");
	/* check for dumb buffers support */
	uint64_t has_dumb;
	if (drmGetCap(mi->fb, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 || !has_dumb){
		/* not supported */
		ALOGV("DRMFB_init device does not support dumb buffers");
		goto fail;
	}
	/* get di->resources */
	ALOGV("DRMFB_init get di->resources");
	di->res = drmModeGetResources(mi->fb);
	if (!di->res){
		ALOGV("DRMFB_init failed to get drm di->resources");
		goto fail;
	}
	ALOGV("DRMFB_init check for used connectors");
	/* check for used connectors */
	if (di->res->count_crtcs > 0 && di->res->count_connectors > 0){
		for (i=0; i < di->res->count_connectors; i++){
			di->con = drmModeGetConnector(mi->fb, di->res->connectors[i]);
			if (di->con){
				if ((di->con->count_modes > 0) && (di->con->connection == DRM_MODE_CONNECTED)){
					break;
				}
				drmModeFreeConnector(di->con);
			}
		}
	}
	
	ALOGV("DRMFB_init look for main display, if any specified");
	/* look for LVDS/eDP/DSI connectors, if any. Those are the main screens. */
	drmModeConnector *main_con = NULL;
	for (i=0; i < di->res->count_connectors; i++){
		main_con = drmModeGetConnector(mi->fb, di->res->connectors[i]);
		if (main_con){
			if ((main_con->connection == DRM_MODE_CONNECTED) &&
				(main_con->count_modes > 0) &&
				(main_con->connector_type == DRM_MODE_CONNECTOR_LVDS || 
				main_con->connector_type == DRM_MODE_CONNECTOR_eDP ||
				main_con->connector_type == DRM_MODE_CONNECTOR_DSI)){
				break;
			}
			/* try next connector */
			drmModeFreeConnector(main_con);
			main_con = NULL;
		}
	}
	/* use found connector, if any. Otherwise, use first found before */
	if (main_con){
		ALOGV("DRMFB_init main display specified=%p", main_con);
		di->con = main_con;
	}
	/* get preferred screen mode */
	ALOGV("DRMFB_init get preferred mode");
	int mode_index = 0, modes;
	for (modes = 0; modes < di->con->count_modes; modes++){
		if (di->con->modes[modes].type & DRM_MODE_TYPE_PREFERRED){
			mode_index = modes;
			break;
		}
	}
	/* find crtc for connector */
	ALOGV("DRMFB_init get crtc for connector");
	int crtc_id=-1;
	if (di->con->encoder_id) enc = drmModeGetEncoder(mi->fb, di->con->encoder_id);
	if (enc && enc->crtc_id){
		crtc_id = enc->crtc_id;
		drmModeFreeEncoder(enc);
		di->crtc = drmModeGetCrtc(mi->fb, crtc_id);
	}
	if (!di->crtc){
		for (i=0; i < di->con->count_encoders; i++){
			enc = drmModeGetEncoder(mi->fb, di->con->encoders[i]);

			if (enc){
				for (j=0; j < di->res->count_crtcs; j++){
					if (!(enc->possible_crtcs & (1 << j))){
						continue;
					}
					crtc_id = di->res->crtcs[j];
					break;
				}
				if (crtc_id >= 0){
					drmModeFreeEncoder(enc);
					di->crtc = drmModeGetCrtc(mi->fb, crtc_id);
				}
			}
		}
	}
	/* set crtc mode & save size info */
	ALOGV("DRMFB_init set crtc");
	di->crtc->mode = di->con->modes[mode_index];
	/* create dumb buffers */
	ALOGV("DRMFB_init create dumb buffers");
	for (i=0; i<2; i++){
		retval = DRMFB_mkdumb(mi, di->crtc->mode.hdisplay, di->crtc->mode.vdisplay, i);
		if (!retval){
			ALOGV("DRMFB_init failed to create dumb buffer #%i", i);
			drmModeFreeResources(di->res);
			goto fail;
		}
	}
	/* use first buffer for crtc */
	ALOGV("DRMFB_init set first buffer for crtc");
	retval = drmModeSetCrtc(mi->fb, di->crtc->crtc_id,
						 di->buffer_id[0],
						 0, 0, // x, y
						 &di->con->connector_id,
						 1, // connector_count
						 &di->crtc->mode);
	if (retval){ 
		ALOGV("DRMFB_init drmModeSetCrtc failed (%d)", retval);
	}
	/* set LIBAROMA_FBP values */
	me->w = di->crtc->mode.hdisplay;
	me->h = di->crtc->mode.vdisplay;
	me->sz = me->w * me->h;
	me->double_buffer = 1;
	/* set internal values */
	mi->buffer = di->buffer[0];
	mi->line = di->buffer_stride[0];
	mi->depth = 16;
	mi->pixsz = 2;
	mi->fb_sz = di->buffer_sz[0];
	//mi->stride = mi->line/me->w;
	mi->stride = (mi->line - (me->w * mi->pixsz));
	/* dump display info */
	DRMFB_dump(mi, di);
	return 1;
fail:
	/*  just use the release callback */
	ALOGV("DRMFB_init something failed, releasing");
	DRMFB_release(me);
	return 0;
} /* End of DRMFB_init */

/*
 * Function		: DRMFB_start_post
 * Return Value: byte
 * Descriptions: start post
 */
byte DRMFB_start_post(LIBAROMA_FBP me){
	if (me == NULL) {
		return 0;
	}
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
	libaroma_mutex_lock(mi->mutex);
	return 1;
} /* End of DRMFB_start_post */

/*
 * Function		: DRMFB_post
 * Return Value: byte
 * Descriptions: post
 */
byte DRMFB_post(
	LIBAROMA_FBP me, wordp __restrict src,
	int dx, int dy, int dw, int dh,
	int sx, int sy, int sw, int sh
	){
	if (me == NULL) {
		return 0;
	}
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
	/* DRM doesn't allow to update regions, so we must blit the entire buffer */
	libaroma_blt_align16(
		(wordp) mi->buffer,
		src,
		me->w, me->h,
		mi->stride,
		0
	);
	return 1;
} /* End of DRMFB_post */

/*
 * Function		: DRMFB_end_post
 * Return Value: byte
 * Descriptions: end post
 */
byte DRMFB_end_post(LIBAROMA_FBP me){
	if (me == NULL) {
		return 0;
	}
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
	DRMFB_flush(me);
	libaroma_mutex_unlock(mi->mutex);
	return 1;
} /* End of DRMFB_end_post */

/*
 * Function		: DRMFB_flush
 * Return Value: byte
 * Descriptions: flush display content
 */
byte DRMFB_flush(LIBAROMA_FBP me){
	if (me == NULL) {
		return 0;
	}
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
	DRMFB_INTERNALP di = (DRMFB_INTERNALP) mi->backend;
	/* tell crtc to use already drawn buffer */
	int ret = drmModePageFlip(mi->fb, di->crtc->crtc_id,
						  di->buffer_id[di->current_buffer], 0, NULL);
	if (ret < 0){
		ALOGV("DRMFB_flush page flip failed (%d)", ret);
		return 0;
	}
	/* switch current buffer in order to draw to the background one */
	di->current_buffer = 1-di->current_buffer;
	mi->buffer = di->buffer[di->current_buffer];
	return 1;
} /* End of DRMFB_flush */

/*
 * Function		: DRMFB_release
 * Return Value: void
 * Descriptions: release dri device
 */
void DRMFB_release(LIBAROMA_FBP me){
	if (me == NULL) {
		return;
	}
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
	DRMFB_INTERNALP di = (DRMFB_INTERNALP) mi->backend;
	if (di){
		/* free drm di->resources */
		ALOGV("DRMFB_release free drm resources");
		drmModeFreeResources(di->res);
		/* disable crtc */
		if (di->crtc){
			ALOGV("DRMFB_release disabling crtc");
			drmModeSetCrtc(mi->fb, di->crtc->crtc_id, 0, 0, 0, NULL, 0, NULL);
		}
		/* delete dumb buffers */
		int i;
		for (i=2; i>0; i--){
			ALOGV("DRMFB_release deleting buffer #%d", i);
			DRMFB_deldumb(mi, i);
		}
		mi->buffer=NULL;
		/* release crtc & connector */
		ALOGV("DRMFB_release releasing crtc");
		ALOGV("DRMFB_release releasing connector");
		drmModeFreeCrtc(di->crtc);
		drmModeFreeConnector(di->con);
		/* release internal */
		ALOGV("DRMFB_release releasing internal");
		free(di);
		mi->backend = NULL;
	}
} /* End of DRMFB_release */

/*
 * Function		: DRMFB_mkdumb
 * Return Value: int
 * Descriptions: create & map dumb buffer
 */
static int DRMFB_mkdumb(LINUXFBDR_INTERNALP mi, int w, int h, int i){
	DRMFB_INTERNALP di = (DRMFB_INTERNALP) mi->backend;
	ALOGV("DRMFB_mkdumb create dumb buffer #%d (%dx%d)", i, w, h);
	/* allocate & setup create request struct */
	struct drm_mode_create_dumb create_dumb={0};
	create_dumb.width = w;
	create_dumb.height = h;
	create_dumb.bpp = 16;
	create_dumb.flags = 0;
	/* request dumb buffer with the info above & fill if done */
	ALOGV("DRMFB_mkdumb send create ioctl");
	int retval = drmIoctl(mi->fb, DRM_IOCTL_MODE_CREATE_DUMB, &create_dumb);
	if (retval){
		ALOGV("DRMFB_mkdumb create dumb buffer failed (%d)", retval);
		return 0;
	}
	di->buffer_id[i] = create_dumb.handle;
	/* add buffer to DRM */
	ALOGV("DRMFB_mkdumb add fb to KMS");
	uint32_t handles[4]={0}, pitches[4]={0}, offsets[4]={0};
	handles[0] = create_dumb.handle;
	pitches[0] = create_dumb.pitch;
	uint32_t pixel_format;
	if (libaroma_config()->gfx_override_rgb){
		pixel_format = DRM_FORMAT_BGR565;
	}
	else {
		pixel_format = DRM_FORMAT_RGB565;
	}
	retval = drmModeAddFB2(mi->fb, w, h, pixel_format, handles, pitches, offsets, &(di->buffer_id[i]), 0);
	/*retval = drmModeAddFB(mi->fb, w, h, 16, 16,
			create_dumb.pitch, create_dumb.handle, 
			&di->buffer_id[i]);*/
	if (retval){
		/* failed to add fb to KSM */
		ALOGV("DRMFB_mkdumb drmModeAddFB failed (%d)", retval);
		return 0;
	}
	/* get dumb map information to be used */
	struct drm_mode_map_dumb map_dumb={0};
	map_dumb.handle = create_dumb.handle;
	ALOGV("DRMFB_mkdumb request map info");
	retval = drmIoctl(mi->fb, DRM_IOCTL_MODE_MAP_DUMB, &map_dumb);
	if (retval){
		/* failed to request info */
		ALOGV("DRMFB_mkdumb request map info failed (%d)", retval);
		return 0;
	}
	/* map dumb buffer using obtained info */
	ALOGV("DRMFB_mkdumb mmap buffer");
	di->buffer[i] = mmap(NULL, create_dumb.size, PROT_READ|PROT_WRITE, MAP_SHARED, mi->fb, map_dumb.offset);
	if (di->buffer[i] == MAP_FAILED){
		/* failed to mmap */
		ALOGV("DRMFB_mkdumb mmap failed");
		return 0;
	}
	di->buffer_sz[i]=create_dumb.size;
	di->buffer_stride[i]=create_dumb.pitch;
	return 1;
} /* End of DRMFB_mkdumb */

/*
 * Function		: DRMFB_deldumb
 * Return Value: void
 * Descriptions: delete dumb buffer
 */
static void DRMFB_deldumb(LINUXFBDR_INTERNALP mi, int i){
	DRMFB_INTERNALP di = (DRMFB_INTERNALP) mi->backend;
	ALOGV("DRMFB_deldumb deleting buffer #%d", i);
	struct drm_mode_destroy_dumb del_dumb={0};

	/* unmap buffer */
	if (di->buffer_sz[i]){
		ALOGV("DRMFB_deldumb unmapping buffer");
		munmap(di->buffer[i], di->buffer_sz[i]);
	}

	/* delete framebuffer */
	ALOGV("DRMFB_deldumb removing buffer from KMS");
	drmModeRmFB(mi->fb, di->buffer_id[i]);

	/* delete dumb buffer */
	del_dumb.handle = di->buffer_id[i];
	ALOGV("DRMFB_deldumb destroying dumb buffer");
	drmIoctl(mi->fb, DRM_IOCTL_MODE_DESTROY_DUMB, &del_dumb);
} /* End of DRMFB_deldumb */

/*
 * Function		: LINUXFBDR_dump
 * Return Value: void
 * Descriptions: dump framebuffer informations
 */
static void DRMFB_dump(LINUXFBDR_INTERNALP mi, DRMFB_INTERNALP di){
	ALOGI("DRM DRIVER INFORMATIONS:");
    ALOGV("RES");
    ALOGV(" count_fbs           : %i", di->res->count_fbs);
    ALOGV(" count_crtcs         : %i", di->res->count_crtcs);
    ALOGV(" count_connectors    : %i", di->res->count_connectors);
    ALOGV(" count_encoders      : %i", di->res->count_encoders);
    ALOGV(" min_width           : %i", di->res->min_width);
    ALOGV(" max_width           : %i", di->res->max_width);
    ALOGV(" min_height          : %i", di->res->min_height);
    ALOGV(" max_height          : %i", di->res->max_height);
    ALOGV("CRTC");
    ALOGV(" id                  : %i", di->crtc->crtc_id);
    ALOGV(" x                   : %i", di->crtc->x);
    ALOGV(" y                   : %i", di->crtc->y);
    ALOGV(" width               : %i", di->crtc->width);
    ALOGV(" height              : %i", di->crtc->height);
    ALOGV("MODE");
    ALOGI(" hdisplay            : %i", di->crtc->mode.hdisplay);
    ALOGI(" vdisplay            : %i", di->crtc->mode.vdisplay);
    ALOGV(" htotal              : %i", di->crtc->mode.htotal);
    ALOGV(" vtotal              : %i", di->crtc->mode.vtotal);
    ALOGV(" clock               : %i", di->crtc->mode.clock);
    ALOGV(" vrefresh            : %i", di->crtc->mode.vrefresh);
    ALOGV(" name                : %s", di->crtc->mode.name);
    ALOGI("DUMB FB");
    ALOGI(" depth               : %i", mi->depth);
    ALOGI(" pixsz               : %i", mi->pixsz);
    ALOGI(" mem size            : %i", mi->fb_sz);
    ALOGI(" line size           : %i", mi->line);
    ALOGV(" stride              : %i", mi->stride);
} /* End of LINUXFBDR_dump */

#ifdef __cplusplus
}
#endif

#endif /* LIBAROMA_CONFIG_NODRM */
#endif /* __libaroma_linux_fb_drm_driver_c__ */
