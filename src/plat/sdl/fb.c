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
 * Description : SDL framebuffer driver
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 26/01/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_sdl_fb_driver_c__
#define __libaroma_sdl_fb_driver_c__

/*
 * needed headers
 */
#include <aroma_internal.h>
#ifdef LIBAROMA_PLATFORM_SDL2
	#include <SDL2/SDL.h>
#else
	#include <SDL/SDL.h>
#endif

/*
 * structure : internal framebuffer data
 */
typedef struct {
#ifdef LIBAROMA_PLATFORM_SDL2
	SDL_Window* window;									/* window handle */
	SDL_Renderer* renderer;								/* window renderer */
	SDL_Texture* texture;								/* surface texture */
#else
	SDL_Surface* window;								/* sdl window surface handle */
#endif
	int			fb_sz;									/* framebuffer memory size */
	voidp		buffer;									/* direct buffer */
	int			stride;									/* stride size */
	int			line;									/* line size */
	byte		depth;									/* color depth */
	byte		pixsz;									/* memory size per pixel */
	byte		rgb_pos[6];								/* framebuffer 32bit rgb position */

	LIBAROMA_MUTEX	mutex;
} SDLFBDR_INTERNAL, * SDLFBDR_INTERNALP;

/*
 * Function		: SDLFBDR_flush
 * Return Value: byte
 * Descriptions: flush content into display & wait for vsync
 */
byte SDLFBDR_flush(LIBAROMA_FBP me) {
	if (me == NULL) {
		return 0;
	}
	SDLFBDR_INTERNALP mi = (SDLFBDR_INTERNALP) me->internal;
#ifdef LIBAROMA_PLATFORM_SDL2
	if (SDL_UpdateTexture(mi->texture, NULL, (const void*) mi->buffer, mi->line)<0){
		ALOGE("SDLFBDR_flush failed to update texture (%s)", SDL_GetError());
		return 0;
	}
	if (SDL_RenderCopy(mi->renderer, mi->texture, NULL, NULL)<0){
		ALOGE("SDLFBDR_flush failed to copy texture to renderer (%s)", SDL_GetError());
		return 0;
	}
	SDL_RenderPresent(mi->renderer);
#else
	SDL_Flip(mi->window);
#endif
	return 1;
} /* End of SDLFBDR_flush */

/*
 * Function		: SDLFBDR_start_post
 * Return Value: byte
 * Descriptions: start post
 */
byte SDLFBDR_start_post(LIBAROMA_FBP me){
	if (me == NULL) {
		return 0;
	}
	SDLFBDR_INTERNALP mi = (SDLFBDR_INTERNALP) me->internal;
	libaroma_mutex_lock(mi->mutex);
	return 1;
} /* End of SDLFBDR_start_post */

/*
 * Function		: SDLFBDR_post
 * Return Value: byte
 * Descriptions: post data to display buffer
 */
byte SDLFBDR_post(
	LIBAROMA_FBP me, wordp __restrict src,
	int dx, int dy, int dw, int dh,
	int sx, int sy, int sw, int sh
	){
	if (me == NULL) {
		return 0;
	}
	SDLFBDR_INTERNALP mi = (SDLFBDR_INTERNALP) me->internal;
	int sstride = (sw - dw) * 2;
	int dstride = (mi->line - (dw * mi->pixsz));
	wordp copy_dst =
		(wordp) (mi->buffer+(mi->line * dy)+(dx * mi->pixsz));
	wordp copy_src =
		(wordp) (src + (sw * sy) + sx);
	libaroma_blt_align16(
		copy_dst,
		copy_src,
		dw, dh,
		dstride,
		sstride
	);
	return 1;
} /* En of SDLFBDR_post */

/*
 * Function		: SDLFBDR_end_post
 * Return Value: byte
 * Descriptions: end post
 */
byte SDLFBDR_end_post(LIBAROMA_FBP me){
	if (me == NULL) {
		return 0;
	}
	SDLFBDR_INTERNALP mi = (SDLFBDR_INTERNALP) me->internal;
	SDLFBDR_flush(me);
	libaroma_mutex_unlock(mi->mutex);
	return 1;
} /* End of SDLFBDR_end_post */

/*
 * Function		: SDLFBDR_release
 * Return Value: void
 * Descriptions: release framebuffer driver
 */
void SDLFBDR_release(LIBAROMA_FBP me) {
	if (me==NULL) {
		return;
	}
	SDLFBDR_INTERNALP mi = (SDLFBDR_INTERNALP) me->internal;
	if (mi==NULL){
		return;
	}
#ifdef LIBAROMA_PLATFORM_SDL2
	/* release texture, buffer & renderer */
	free(mi->buffer);
	SDL_DestroyTexture(mi->texture);
	SDL_DestroyWindow(mi->window);
#endif
	SDL_Quit();

	/* destroy mutex & cond */
	libaroma_mutex_free(mi->mutex);

	/* free internal data */
	ALOGV("SDLFBDR free internal data");
	free(me->internal);
} /* End of SDLFBDR_release */

/*
 * Function		: SDLFBDR_init
 * Return Value: byte
 * Descriptions: init framebuffer
 */
byte SDLFBDR_init(LIBAROMA_FBP me) {
	ALOGV("SDLFBDR initialized internal data");

	/* allocating internal data */
	SDLFBDR_INTERNALP mi = (SDLFBDR_INTERNALP) calloc(sizeof(SDLFBDR_INTERNAL),1);
	if (!mi) {
		ALOGE("SDLFBDR calloc internal data - memory error");
		return 0;
	}

	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		ALOGE("Couldn't init SDL: %s", SDL_GetError());
		free(mi);
		return 0;
	}

	/* set internal address */
	me->internal = (voidp) mi;

	/* set release callback */
	me->release = &SDLFBDR_release;

	/* init mutex & cond */
	libaroma_mutex_init(mi->mutex);
	/* initialize SDL window */
#ifdef LIBAROMA_PLATFORM_SDL2
	mi->window=SDL_CreateWindow(libaroma_config()->sdl_wm_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
								libaroma_config()->sdl_wm_width, libaroma_config()->sdl_wm_height,
								0);
#else
	SDL_WM_SetCaption(libaroma_config()->sdl_wm_title, libaroma_config()->sdl_wm_title);
	mi->window = SDL_SetVideoMode(libaroma_config()->sdl_wm_width, libaroma_config()->sdl_wm_height, 16, SDL_HWSURFACE);
	if(!mi->window){
		ALOGE("SDLFBDR could not create SDL hardware-accelerated window");
		mi->window = SDL_SetVideoMode (libaroma_config()->sdl_wm_width, libaroma_config()->sdl_wm_height, 16, SDL_SWSURFACE);
	}
#endif
	if(!mi->window) {
		ALOGE("SDLFBDR could not create SDL window (%s)", SDL_GetError());
		goto error;
	}
#ifdef LIBAROMA_PLATFORM_SDL2
	/* initialize window renderer */
	mi->renderer=SDL_CreateRenderer(mi->window, -1, 0);
	if(!mi->renderer) {
		ALOGE("SDLFBDR could not create SDL renderer (%s)", SDL_GetError());
		goto error;
	}
#endif

	if (libaroma_config()->sdl_nocursor==1){
		ALOGV("SDLFBDR disable SDL cursor");
		SDL_ShowCursor(SDL_DISABLE);
	}

	/* set libaroma framebuffer instance values */
#ifdef LIBAROMA_PLATFORM_SDL2
	int win_w, win_h;
	SDL_GetRendererOutputSize(mi->renderer, &win_w, &win_h);
	/* create SDL2 texture */
	mi->texture = SDL_CreateTexture(mi->renderer,
									SDL_PIXELFORMAT_RGB565,
									SDL_TEXTUREACCESS_STREAMING,
									win_w, win_h);
	if (mi->texture==NULL){
		ALOGE("SDLFBDR failed to create texture (%s)", SDL_GetError());
		goto error;
	}
	me->w  = win_w;				/* width */
	me->h  = win_h;				/* height */
#else
	me->w = mi->window->w;		/* width */
	me->h = mi->window->h;		/* height */
#endif
	me->sz= me->w * me->h;		/* width x height */

	/* set internal useful data */
#ifdef LIBAROMA_PLATFORM_SDL2
	/* allocate pixel buffer - 16bpp */
	mi->buffer = (voidp) calloc(win_w*win_h*2, sizeof(byte));
	if(!mi->buffer) {
		ALOGE("SDLFBDR could not allocate pixel buffer");
		goto error;
	}
#else
	/* use window pixel buffer */
	mi->buffer		= mi->window->pixels;
#endif
	/* on SDL it's always 16bpp, 2 bytes per pixel */
	mi->depth		= 16;
	mi->pixsz		= 2;
	mi->line		= me->w * mi->pixsz;
	mi->fb_sz		=(me->sz * mi->pixsz);
	/* no stride/padding on SDL */
	mi->stride		= 0;

	/* swap buffer now */
	SDLFBDR_flush(me);

	/* set driver callbacks */
	me->start_post	= &SDLFBDR_start_post;
	me->end_post	= &SDLFBDR_end_post;
	me->post		= &SDLFBDR_post;
	me->snapshoot	= NULL;

	ALOGI("SDL BUFFER INFORMATIONS:");
	ALOGI(" width               : %i", me->w);
	ALOGI(" height              : %i", me->h);
	ALOGI(" bits_per_pixel      : %i", mi->depth);
	ALOGD(" pixelsize           : %i", mi->pixsz);
	ALOGD(" linesize            : %i", mi->line);
	ALOGD(" fb_size             : %i", mi->fb_sz);

	/* init done */
	return 1;
error:
	/* something failed */
#ifdef LIBAROMA_PLATFORM_SDL2
	if (mi->buffer!=NULL){
		free(mi->buffer);
	}
	if (mi->texture!=NULL){
		SDL_DestroyTexture(mi->texture);
	}
	if (mi->window!=NULL){
		SDL_DestroyWindow(mi->window);
	}
#else

#endif
	/* exit SDL */
	SDL_Quit();
	free(mi);
	return 0;
} /* End of SDLFBDR_init */

/*
 * Function		: libaroma_fb_driver_init
 * Return Value: byte
 * Descriptions: init function for libaroma fb
 */
byte libaroma_fb_driver_init(LIBAROMA_FBP me) {
	return SDLFBDR_init(me);
} /* End of libaroma_fb_driver_init */

#endif /* __libaroma_sdl_fb_driver_c__ */
