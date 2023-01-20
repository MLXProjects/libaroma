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
 * Filename		: fb_driver.c
 * Description : linux framebuffer driver
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 26/01/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_linux_fb_driver_c__
#define __libaroma_linux_fb_driver_c__

#include <aroma_internal.h>
#include "fb.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Function		: LINUXFBDR_start_post
 * Return Value: byte
 * Descriptions: start post
 */
byte LINUXFBDR_start_post(LIBAROMA_FBP me){
	if (me == NULL) {
		return 0;
	}
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
	libaroma_mutex_lock(mi->mutex);
	return 1;
}

/*
 * Function		: LINUXFBDR_end_post
 * Return Value: byte
 * Descriptions: end post
 */
byte LINUXFBDR_end_post(LIBAROMA_FBP me){
	if (me == NULL) {
		return 0;
	}
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
	LINUXFBDR_wait_vsync(mi);
	LINUXFBDR_flush(me);
	libaroma_mutex_unlock(mi->mutex);
	return 1;
}

/*
 * Function		: LINUXFBDR_config
 * Return Value: byte
 * Descriptions: config framebuffer
 */
byte LINUXFBDR_config(
	LIBAROMA_FBP me, const char * name, const char * sval, dword dval
){
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
	if (mi==NULL){
		return 0;
	}
	if (strcmp(name,"pointer")==0){
		mi->pointered=1;
		mi->pointer_x=LOWORD(dval);
		mi->pointer_y=HIWORD(dval);

		struct fb_cursor cur;
		cur.enable=1;
		cur.hot.x=mi->pointer_x;
		cur.hot.x=mi->pointer_y;
		cur.set=FB_CUR_SETPOS;
		ioctl(mi->fb,FBIO_CURSOR,&cur);
	}
	return 0;
}

/*
 * Function		: LINUXFBDR_init
 * Return Value: byte
 * Descriptions: init framebuffer
 */
byte LINUXFBDR_init(LIBAROMA_FBP me) {
	ALOGV("LINUXFBDR initialized internal data");

	/* allocating internal data */
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) calloc(sizeof(LINUXFBDR_INTERNAL),1);
	if (!mi) {
		ALOGE("LINUXFBDR calloc internal data - memory error");
		return 0;
	}

	/* set internal address */
	me->internal = (voidp) mi;

	/* set release callback */
	me->release = &LINUXFBDR_release;

	/* init mutex & cond */
	libaroma_mutex_init(mi->mutex);

	/* open framebuffer device */
	mi->fb = open(LINUXFBDR_DEVICE, O_RDWR, 0);
	if (mi->fb < 1) {
		/* if not works, try non android standard device path */
		mi->fb = open(LINUXFBDR_DEVICE_NON_ANDROID, O_RDWR, 0);
	}
	if (mi->fb < 1) {
		/* cannot find device */
		ALOGE("LINUXFBDR no framebuffer device");
		goto error; /* exit if error */
	}

	/* get framebuffer var & fix data */
	ioctl(mi->fb, FBIOGET_FSCREENINFO, &mi->fix); /* fix info */
	ioctl(mi->fb, FBIOGET_VSCREENINFO, &mi->var); /* var info */

	/* dump info */
	LINUXFBDR_dump(mi);

	/* set libaroma framebuffer instance values */
	me->w	= mi->var.xres;	/* width */
	me->h	= mi->var.yres;	/* height */
	me->sz	= me->w*me->h;	/* width x height */

	if (QCOMFB_init(me)){
		/* qcom fb */
		me->start_post	= &QCOMFB_start_post;
		me->end_post	= &QCOMFB_end_post;
		me->post		= &QCOMFB_post;
		me->snapshoot	= NULL;
		ALOGI("using qcom framebuffer driver");
	}
	else{
		/* it's not qcom */
		ALOGI("not using qcom framebuffer driver");

		if ((mi->var.bits_per_pixel != 32) && (mi->var.bits_per_pixel != 16)) {
			/* non 32/16bit colorspace is not supported */
			ALOGE("LINUXFBDR bits_per_pixel=%i not supported",
				mi->var.bits_per_pixel);
			goto error;
		}

		/* init features - double buffer, vsync */
		LINUXFBDR_init_features(me);

		/* set internal useful data */
		mi->line = mi->fix.line_length;		/* line memory size */
		mi->depth= mi->var.bits_per_pixel;	/* color depth */
		mi->pixsz= mi->depth >> 3 ;			/* pixel size per byte */
		mi->fb_sz= (mi->var.xres_virtual * mi->var.yres_virtual * mi->pixsz);

		if (mi->fix.smem_len < (unsigned int) mi->fb_sz){
			/* smem_len is invalid */
			ALOGW("LINUXFBDR smem_len(%i) < fb_sz(%i)", mi->fix.smem_len, mi->fb_sz);
			//goto error;
		}
		/* map buffer */
		ALOGV("LINUXFBDR mmap Framebuffer Memory");
		mi->buffer = mmap(0, mi->fix.smem_len,
							PROT_READ | PROT_WRITE, MAP_SHARED,
							mi->fb, 0);

		if (mi->buffer == MAP_FAILED) {
			ALOGE("LINUXFBDR mmap framebuffer memory error");
			goto error;
		}

		/* swap buffer now */
		LINUXFBDR_flush(me);

		if (mi->pixsz == 2) {
			/* not 32bit depth */
			mi->is32 = 0;
			/* init colorspace */
			LINUXFBDR_init_16bit(me);
		}
		else {
			mi->is32 = 1;
			/* init colorspace */
			LINUXFBDR_init_32bit(me);
		}
	}

	/* set config */
	me->config = &LINUXFBDR_config;
	me->setrgb = &LINUXFBDR_setrgbpos;

	/* set dpi */
	LINUXFBDR_set_dpi(me);

	/* ok */
	goto ok;

	/* return */
error:
	free(mi);
	return 0;
ok:
	return 1;
} /* End of LINUXFBDR_init */

/*
 * Function		: LINUXFBDR_release
 * Return Value: void
 * Descriptions: release framebuffer driver
 */
void LINUXFBDR_release(LIBAROMA_FBP me) {
	if (me==NULL) {
		return;
	}
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
	if (mi==NULL){
		return;
	}

	if (mi->backend!=NULL){
		BACKEND_INTERNALP bi = (BACKEND_INTERNALP) mi->backend;
		if (bi->type == LINUXFBDR_BACKEND_QCOM){
			/* release qcom overlay driver */
			QCOMFB_release(me);
		}
	}

	/* unmap */
	if (mi->buffer!=NULL){
		ALOGV("LINUXFBDR munmap buffer");
		munmap(mi->buffer, mi->fix.smem_len);
	}

	/* close fb */
	if (mi->fb >= 0){
		ALOGV("LINUXFBDR close fb-fd");
		close(mi->fb);
	}

	/* destroy mutex & cond */
	libaroma_mutex_free(mi->mutex);

	/* free internal data */
	ALOGV("LINUXFBDR free internal data");
	free(me->internal);
} /* End of LINUXFBDR_release */

/*
 * Function		: LINUXFBDR_swap_buffer
 * Return Value: void
 * Descriptions: swap back buffer
 */
void LINUXFBDR_swap_buffer(LINUXFBDR_INTERNALP mi){
	mi->current_buffer = mi->buffer + (mi->var.yoffset * mi->fix.line_length);
	if ((mi->double_buffering)&&(mi->var.yoffset==0)){
		mi->var.yoffset = mi->var.yres;
	}
	else{
		mi->var.yoffset=0;
	}
} /* End of LINUXFBDR_swap_buffer */

/*
 * Function		: LINUXFBDR_wait_vsync
 * Return Value: void
 * Descriptions: wait for vsync
 */
void LINUXFBDR_wait_vsync(LINUXFBDR_INTERNALP mi){
	if (mi->is_omap){
		int s=0;
		mi->last_vsync=ioctl(mi->fb, OMAPFB_WAITFORVSYNC, &s);
	}
	else{
		mi->last_vsync=ioctl(mi->fb, FBIO_WAITFORVSYNC, 0);
	}
} /* End of LINUXFBDR_wait_vsync */

/*
 * Function		: LINUXFBDR_flush
 * Return Value: byte
 * Descriptions: flush content into display & wait for vsync
 */
byte LINUXFBDR_flush(LIBAROMA_FBP me) {
	if (me == NULL) {
		return 0;
	}
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;

	LINUXFBDR_swap_buffer(mi);
	
	if (ioctl(mi->fb, FBIOPAN_DISPLAY, &mi->var)!=0){
		if (mi->last_vsync==0){
			mi->var.activate = FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE;
		}
		else{
			mi->var.activate = FB_ACTIVATE_VBL;
		}
		ioctl(mi->fb, FBIOPUT_VSCREENINFO, &mi->var);
	}
	return 1;
} /* End of LINUXFBDR_flush */

/*
 * Function		: LINUXFBDR_init_features
 * Return Value: void
 * Descriptions: init framebuffer features
 */
void LINUXFBDR_init_features(LIBAROMA_FBP me) {
	if (me==NULL) {
		return;
	}
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
	if (mi==NULL){
		return;
	}
	int res=0;

	/* set non interlanced */
	mi->var.vmode = FB_VMODE_NONINTERLACED;

	/* request double buffer */
	mi->double_buffering=0;
	if (mi->var.yres_virtual<mi->var.yres*2){
		mi->var.yres_virtual=mi->var.yres*2;
		mi->var.activate = FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE;
		ioctl(mi->fb, FBIOPUT_VSCREENINFO, &mi->var);

		/* update vars */
		ioctl(mi->fb, FBIOGET_FSCREENINFO, &mi->fix);
		ioctl(mi->fb, FBIOGET_VSCREENINFO, &mi->var);
		if (mi->var.yres_virtual>=mi->var.yres*2){
			/* support double buffering */
			mi->double_buffering=1;
		}
	}
	else{
		mi->double_buffering=1;
	}
	me->double_buffer=mi->double_buffering;
	ALOGV("LINUXFBDR Double Buffering = %s",mi->double_buffering?"yes":"no");

	/* activate vsync - universal */
	mi->var.sync=FB_SYNC_VERT_HIGH_ACT;
	mi->var.activate = FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE;
	res = ioctl(mi->fb, FBIOPUT_VSCREENINFO, &mi->var);
	ALOGV("LINUXFBDR FB_SYNC_VERT_HIGH_ACT = %i",res);

	/* update vars */
	ioctl(mi->fb, FBIOGET_FSCREENINFO, &mi->fix);
	ioctl(mi->fb, FBIOGET_VSCREENINFO, &mi->var);

	/* enable omapfb vsync */
	mi->is_omap=0;
	if (mi->var.sync!=FB_SYNC_VERT_HIGH_ACT){
		if (!strncmp(mi->fix.id, "omapfb", strlen("omapfb"))){
			int state=1;
			res=ioctl(mi->fb, OMAPFB_ENABLEVSYNC, &state);
			ALOGV("LINUXFBDR OMAPFB_ENABLEVSYNC = %i",res);
			if (res==0){
				mi->is_omap=1;
			}
		}
	}
} /* End of LINUXFBDR_init_features */

/*
 * Function		: LINUXFBDR_set_dpi
 * Return Value: void
 * Descriptions: set dpi
 */
void LINUXFBDR_set_dpi(LIBAROMA_FBP me) {
	if (me==NULL) {
		return;
	}
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
	me->dpi = 0;
	int dpi_fallback = floor(MIN(mi->var.xres,mi->var.yres)/160) * 80;
	if ((mi->var.width<= 0)||(mi->var.height <= 0)) {
		/* phone dpi */
		me->dpi = dpi_fallback;
	}
	else{
		/* calculate dpi */
		me->dpi = round(mi->var.xres / (mi->var.width * 0.039370) / 80) * 80;
	}
	if ((me->dpi<120)||(me->dpi>960)){
		me->dpi = dpi_fallback;
	}
#ifdef LIBAROMA_PROP_GETDPI
	/* android dpi from default.prop/build.prop */
	char * sf_lcd=libaroma_getprop("ro.sf.lcd_density",
		libaroma_stream_file("/default.prop"), 1);
	if (sf_lcd==NULL){
		/* try /system/build.prop */
		sf_lcd=libaroma_getprop("ro.sf.lcd_density",
			libaroma_stream_file("/system/build.prop"), 1);
	}
	if (sf_lcd!=NULL){
		int new_dpi = atoi(sf_lcd);
		free(sf_lcd);
		if ((new_dpi>=120)&&(new_dpi<=980)){
			ALOGI("android getprop ro.sf.lcd_density: %i - OK",new_dpi);
			me->dpi=new_dpi;
		}
		else{
			ALOGI("android getprop ro.sf.lcd_density: %i - INVALID",new_dpi);
		}
	}
	else{
		ALOGI("android getprop ro.sf.lcd_density not found");
	}
#endif /* LIBAROMA_PROP_GETDPI */
} /* End of LINUXFBDR_set_dpi */

/*
 * Function		: LINUXFBDR_dump
 * Return Value: void
 * Descriptions: dump framebuffer informations
 */
void LINUXFBDR_dump(LINUXFBDR_INTERNALP mi) {
	ALOGI("FRAMEBUFFER INFORMATIONS:");
	ALOGI("VAR");
	ALOGI(" xres                : %i", mi->var.xres);
	ALOGI(" yres                : %i", mi->var.yres);
	ALOGI(" xres_virtual        : %i", mi->var.xres_virtual);
	ALOGI(" yres_virtual        : %i", mi->var.yres_virtual);
	ALOGI(" xoffset             : %i", mi->var.xoffset);
	ALOGI(" yoffset             : %i", mi->var.yoffset);
	ALOGI(" bits_per_pixel      : %i", mi->var.bits_per_pixel);
	ALOGI(" grayscale           : %i", mi->var.grayscale);
	ALOGI(" red                 : %i, %i, %i",
		mi->var.red.offset, mi->var.red.length, mi->var.red.msb_right);
	ALOGI(" green               : %i, %i, %i",
		mi->var.green.offset, mi->var.green.length, mi->var.red.msb_right);
	ALOGI(" blue                : %i, %i, %i",
		mi->var.blue.offset, mi->var.blue.length, mi->var.red.msb_right);
	ALOGI(" transp              : %i, %i, %i",
		mi->var.transp.offset, mi->var.transp.length, mi->var.red.msb_right);
	ALOGI(" nonstd              : %i", mi->var.nonstd);
	ALOGI(" activate            : %i", mi->var.activate);
	ALOGI(" height              : %i", mi->var.height);
	ALOGI(" width               : %i", mi->var.width);
	ALOGI(" accel_flags         : %i", mi->var.accel_flags);
	ALOGI(" pixclock            : %i", mi->var.pixclock);
	ALOGI(" left_margin         : %i", mi->var.left_margin);
	ALOGI(" right_margin        : %i", mi->var.right_margin);
	ALOGI(" upper_margin        : %i", mi->var.upper_margin);
	ALOGI(" lower_margin        : %i", mi->var.lower_margin);
	ALOGI(" hsync_len           : %i", mi->var.hsync_len);
	ALOGI(" vsync_len           : %i", mi->var.vsync_len);
	ALOGI(" sync                : %i", mi->var.sync);
	ALOGI(" rotate              : %i", mi->var.rotate);

	ALOGI("FIX");
	ALOGI(" id                  : %s", mi->fix.id);
	ALOGI(" smem_len            : %i", mi->fix.smem_len);
	ALOGI(" type                : %i", mi->fix.type);
	ALOGI(" type_aux            : %i", mi->fix.type_aux);
	ALOGI(" visual              : %i", mi->fix.visual);
	ALOGI(" xpanstep            : %i", mi->fix.xpanstep);
	ALOGI(" ypanstep            : %i", mi->fix.ypanstep);
	ALOGI(" ywrapstep           : %i", mi->fix.ywrapstep);
	ALOGI(" line_length         : %i", mi->fix.line_length);
	ALOGI(" accel               : %i", mi->fix.accel);
	ALOGI(" line size           : %i", mi->line);
} /* End of LINUXFBDR_dump */

/*
 * Function		: LINUXFBDR_setrgbpos
 * Return Value: void
 * Descriptions: set rgbx position
 */
void LINUXFBDR_setrgbpos(LIBAROMA_FBP me, byte r, byte g, byte b) {
	if (me == NULL) {
		return;
	}
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
	/* save color position */
	mi->rgb_pos[0] = r;
	mi->rgb_pos[1] = g;
	mi->rgb_pos[2] = b;
	mi->rgb_pos[3] = r >> 3;
	mi->rgb_pos[4] = g >> 3;
	mi->rgb_pos[5] = b >> 3;
}

/*
 * Function		: libaroma_fb_driver_init
 * Return Value: byte
 * Descriptions: init function for libaroma fb
 */
byte libaroma_fb_driver_init(LIBAROMA_FBP me) {
	return LINUXFBDR_init(me);
} /* End of libaroma_fb_driver_init */

#ifdef __cplusplus
}
#endif
#endif /* __libaroma_linux_fb_driver_c__ */
