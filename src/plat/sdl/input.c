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
 * Filename		: hid_driver.c
 * Description : linux input hid driver
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 26/01/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_linux_hid_driver_c__
#define __libaroma_linux_hid_driver_c__
#include <aroma_internal.h>

/*
 * UNIVERSAL DEVICE - INPUT DRIVER
 *	 Using SDL Input driver
 *	 Prefix : SDLHIDRV_
 */

/*
 * headers
 */
#include <unistd.h>
#ifdef LIBAROMA_PLATFORM_SDL2
	#include <SDL2/SDL.h>
#else
	#include <SDL/SDL.h>
#endif

static byte _SDLHIDRV_initialized = 0;
static byte _sdl_mouse_down = 0;

/*
 * forward functions
 */
void SDLHIDRV_release(
		LIBAROMA_HIDP me);
byte SDLHIDRV_getinput(
		LIBAROMA_HIDP me,
		LIBAROMA_HID_EVENTP dest_ev);

/*
 * function : init input device
 */
byte SDLHIDRV_init(
		LIBAROMA_HIDP me) {

	/* set driver callbacks */
	me->release		= &SDLHIDRV_release;
	me->getinput	 = &SDLHIDRV_getinput;
	
	/* set initialized */
	_SDLHIDRV_initialized = 1;
	/* ok */
	return 1;
}

/*
 * function : release input driver instance
 */
void SDLHIDRV_release(
		LIBAROMA_HIDP me) {
	/* is input instance initialized ? */
	if (me == NULL) {
		return;
	}
	/* unset initialized */
	_SDLHIDRV_initialized = 0;
}

/*
 * function : get input callback
 */
byte SDLHIDRV_getinput(
		LIBAROMA_HIDP me,
		LIBAROMA_HID_EVENTP dest_ev) {
	SDL_Event event;

	/* polling loop */
	do {
		if(SDL_WaitEvent(&event)) {
			switch(event.type) {
				case SDL_QUIT:
					return LIBAROMA_HID_EV_RET_EXIT;

				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:

					dest_ev->type	= LIBAROMA_HID_EV_TYPE_TOUCH;
					dest_ev->key	= 0;
					dest_ev->x		= event.button.x;
					dest_ev->y		= event.button.y;
					dest_ev->state	= (event.type==SDL_MOUSEBUTTONDOWN?
						LIBAROMA_HID_EV_STATE_DOWN:LIBAROMA_HID_EV_STATE_UP);
					_sdl_mouse_down = dest_ev->state;
					return LIBAROMA_HID_EV_RET_TOUCH;

				case SDL_MOUSEMOTION:
					if (_sdl_mouse_down==LIBAROMA_HID_EV_STATE_DOWN || libaroma_config()->sdl_mousemove){
						dest_ev->type	= LIBAROMA_HID_EV_TYPE_TOUCH;
						dest_ev->key	= 0;
						dest_ev->x		= event.motion.x;
						dest_ev->y		= event.motion.y;
						dest_ev->state	= LIBAROMA_HID_EV_STATE_MOVE;
						return LIBAROMA_HID_EV_RET_TOUCH;
					}
			}
		}
	}
	while (_SDLHIDRV_initialized);

	/* it was exit message */
	ALOGV("SDLHIDRV_getinput Input Driver Already Released");
	return LIBAROMA_HID_EV_RET_EXIT;
}

/*
 * function : libaroma init hid driver
 */
byte libaroma_hid_driver_init(LIBAROMA_HIDP me) {
	return SDLHIDRV_init(me);
}

#endif /* __libaroma_linux_hid_driver_c__ */
