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
byte SDLHIDRV_translate_keyboard(
		LIBAROMA_HID_EVENTP dest_ev,
		SDL_Event *sdl_ev);

/*
 * function : init input device
 */
byte SDLHIDRV_init(
		LIBAROMA_HIDP me) {

	/* set driver callbacks */
	me->release	= &SDLHIDRV_release;
	me->getinput= &SDLHIDRV_getinput;
	
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
				case SDL_MOUSEBUTTONUP:{
					dest_ev->type	= LIBAROMA_HID_EV_TYPE_TOUCH;
					dest_ev->key	= 0;
					dest_ev->x		= event.button.x;
					dest_ev->y		= event.button.y;
					dest_ev->state	= (event.type==SDL_MOUSEBUTTONDOWN)?
						LIBAROMA_HID_EV_STATE_DOWN:LIBAROMA_HID_EV_STATE_UP;
					_sdl_mouse_down = dest_ev->state;
					return LIBAROMA_HID_EV_RET_TOUCH;
				} break;
				case SDL_MOUSEMOTION:{
					if (_sdl_mouse_down==LIBAROMA_HID_EV_STATE_DOWN || libaroma_config()->sdl_mousemove){
						dest_ev->type	= LIBAROMA_HID_EV_TYPE_TOUCH;
						dest_ev->key	= 0;
						dest_ev->x		= event.motion.x;
						dest_ev->y		= event.motion.y;
						dest_ev->state	= LIBAROMA_HID_EV_STATE_MOVE;
						return LIBAROMA_HID_EV_RET_TOUCH;
					}
				} break;
				case SDL_KEYDOWN:
				case SDL_KEYUP:
					return SDLHIDRV_translate_keyboard(dest_ev, &event);
			}
		}
	}
	while (_SDLHIDRV_initialized);

	/* it was exit message */
	ALOGV("SDLHIDRV_getinput Input Driver Already Released");
	return LIBAROMA_HID_EV_RET_EXIT;
}

byte SDLHIDRV_translate_keyboard(
		LIBAROMA_HID_EVENTP dest_ev,
		SDL_Event *sdl_ev){
#ifdef LIBAROMA_PLATFORM_SDL2
	SDL_Keysym *sdl_sym;
#else
	SDL_keysym *sdl_sym;
#endif
	sdl_sym = &(sdl_ev->key.keysym);
	/* dump raw events */
	ALOGRT("RAW KEY: C=%i, S=%i, M=%i",
			sdl_sym->scancode,
			sdl_sym->sym,
			sdl_sym->mod
	);
	/* fill destination event */
	dest_ev->type	= LIBAROMA_HID_EV_TYPE_KEY;
	dest_ev->x		= 0;
	dest_ev->y		= 0;
	dest_ev->state	= (sdl_ev->type==SDL_KEYDOWN)?
		LIBAROMA_HID_EV_STATE_DOWN:LIBAROMA_HID_EV_STATE_UP;

    /* translate key code to aroma key & return code.
     * A-Z keys just set key value and return EV_RET_KEY.
     * multiple keys with same return value may set dest_ev->key = UNKNOWN
     * if it doesn't have a HID_KEY_* related value, or just use a related
     * keycode to differentiate them (e.g. ENTER/KPENTER/SELECT). */
	switch (sdl_sym->sym){
		/* first check keys with dedicated return code */
		case SDLK_POWER:
			dest_ev->key = LIBAROMA_HID_KEY_POWER;
			return LIBAROMA_HID_EV_RET_POWER;
		/* keypad enter is at the top because of it's custom return code */
		case SDLK_KP_ENTER:
			dest_ev->key = LIBAROMA_HID_KEY_KPENTER;
			return LIBAROMA_HID_EV_RET_SELECT;
	#ifdef LIBAROMA_PLATFORM_SDL2
		case SDLK_ENTER:
			dest_ev->key = LIBAROMA_HID_KEY_ENTER;
			return LIBAROMA_HID_EV_RET_SELECT;
		case SDLK_SELECT:
			dest_ev->key = LIBAROMA_HID_KEY_UNKNOWN;
			return LIBAROMA_HID_EV_RET_SELECT;
		case SDLK_VOLUMEDOWN:
			dest_ev->key = LIBAROMA_HID_KEY_VOLUMEDOWN;
			return LIBAROMA_HID_EV_RET_VOLDOWN;
		case SDLK_VOLUMEUP:
			dest_ev->key = LIBAROMA_HID_KEY_VOLUMEUP;
			return LIBAROMA_HID_EV_RET_VOLUP;
	#else
		case SDLK_RETURN:
			dest_ev->key = LIBAROMA_HID_KEY_ENTER;
			return LIBAROMA_HID_EV_RET_SELECT;

	#endif
		case SDLK_MENU:
			dest_ev->key = LIBAROMA_HID_KEY_UNKNOWN;
			return LIBAROMA_HID_EV_RET_MENU;
		case SDLK_ESCAPE:
			dest_ev->key = LIBAROMA_HID_KEY_ESC;
			return LIBAROMA_HID_EV_RET_BACK;
		case SDLK_UP:
			dest_ev->key = LIBAROMA_HID_KEY_UP;
			return LIBAROMA_HID_EV_RET_UP;
		case SDLK_DOWN:
			dest_ev->key = LIBAROMA_HID_KEY_DOWN;
			return LIBAROMA_HID_EV_RET_DOWN;
		/* now check common keys */
		case SDLK_BACKQUOTE:
			dest_ev->key = LIBAROMA_HID_KEY_GRAVE;
			break;
		case SDLK_1:
			dest_ev->key = LIBAROMA_HID_KEY_1;
			break;
		case SDLK_2:
			dest_ev->key = LIBAROMA_HID_KEY_2;
			break;
		case SDLK_3:
			dest_ev->key = LIBAROMA_HID_KEY_3;
			break;
		case SDLK_4:
			dest_ev->key = LIBAROMA_HID_KEY_4;
			break;
		case SDLK_5:
			dest_ev->key = LIBAROMA_HID_KEY_5;
			break;
		case SDLK_6:
			dest_ev->key = LIBAROMA_HID_KEY_6;
			break;
		case SDLK_7:
			dest_ev->key = LIBAROMA_HID_KEY_7;
			break;
		case SDLK_8:
			dest_ev->key = LIBAROMA_HID_KEY_8;
			break;
		case SDLK_9:
			dest_ev->key = LIBAROMA_HID_KEY_9;
			break;
		case SDLK_0:
			dest_ev->key = LIBAROMA_HID_KEY_0;
			break;
		case SDLK_MINUS:
			dest_ev->key = LIBAROMA_HID_KEY_MINUS;
			break;
		case SDLK_EQUALS:
			dest_ev->key = LIBAROMA_HID_KEY_EQUAL;
			break;
		case SDLK_BACKSPACE:
			dest_ev->key = LIBAROMA_HID_KEY_BACKSPACE;
			break;
		case SDLK_TAB:
			dest_ev->key = LIBAROMA_HID_KEY_TAB;
			break;
		case SDLK_q:
			dest_ev->key = LIBAROMA_HID_KEY_Q;
			break;
		case SDLK_w:
			dest_ev->key = LIBAROMA_HID_KEY_W;
			break;
		case SDLK_e:
			dest_ev->key = LIBAROMA_HID_KEY_E;
			break;
		case SDLK_r:
			dest_ev->key = LIBAROMA_HID_KEY_R;
			break;
		case SDLK_t:
			dest_ev->key = LIBAROMA_HID_KEY_T;
			break;
		case SDLK_y:
			dest_ev->key = LIBAROMA_HID_KEY_Y;
			break;
		case SDLK_u:
			dest_ev->key = LIBAROMA_HID_KEY_U;
			break;
		case SDLK_i:
			dest_ev->key = LIBAROMA_HID_KEY_I;
			break;
		case SDLK_o:
			dest_ev->key = LIBAROMA_HID_KEY_O;
			break;
		case SDLK_p:
			dest_ev->key = LIBAROMA_HID_KEY_P;
			break;
		case SDLK_LEFTBRACKET:
			dest_ev->key = LIBAROMA_HID_KEY_LEFTBRACE;
			break;
		case SDLK_RIGHTBRACKET:
			dest_ev->key = LIBAROMA_HID_KEY_RIGHTBRACE;
			break;
		case SDLK_CAPSLOCK:
			dest_ev->key = LIBAROMA_HID_KEY_CAPSLOCK;
			break;
		case SDLK_a:
			dest_ev->key = LIBAROMA_HID_KEY_A;
			break;
		case SDLK_s:
			dest_ev->key = LIBAROMA_HID_KEY_S;
			break;
		case SDLK_d:
			dest_ev->key = LIBAROMA_HID_KEY_D;
			break;
		case SDLK_f:
			dest_ev->key = LIBAROMA_HID_KEY_F;
			break;
		case SDLK_g:
			dest_ev->key = LIBAROMA_HID_KEY_G;
			break;
		case SDLK_h:
			dest_ev->key = LIBAROMA_HID_KEY_H;
			break;
		case SDLK_j:
			dest_ev->key = LIBAROMA_HID_KEY_J;
			break;
		case SDLK_k:
			dest_ev->key = LIBAROMA_HID_KEY_K;
			break;
		case SDLK_l:
			dest_ev->key = LIBAROMA_HID_KEY_L;
			break;
		case SDLK_SEMICOLON:
			dest_ev->key = LIBAROMA_HID_KEY_SEMICOLON;
			break;
		case SDLK_QUOTE:
			dest_ev->key = LIBAROMA_HID_KEY_APOSTROPHE;
			break;
		case SDLK_LSHIFT:
			dest_ev->key = LIBAROMA_HID_KEY_LEFTSHIFT;
			break;
		case SDLK_BACKSLASH:
			dest_ev->key = LIBAROMA_HID_KEY_BACKSLASH;
			break;
		case SDLK_z:
			dest_ev->key = LIBAROMA_HID_KEY_Z;
			break;
		case SDLK_x:
			dest_ev->key = LIBAROMA_HID_KEY_X;
			break;
		case SDLK_c:
			dest_ev->key = LIBAROMA_HID_KEY_C;
			break;
		case SDLK_v:
			dest_ev->key = LIBAROMA_HID_KEY_V;
			break;
		case SDLK_b:
			dest_ev->key = LIBAROMA_HID_KEY_B;
			break;
		case SDLK_n:
			dest_ev->key = LIBAROMA_HID_KEY_N;
			break;
		case SDLK_m:
			dest_ev->key = LIBAROMA_HID_KEY_M;
			break;
		case SDLK_COMMA:
			dest_ev->key = LIBAROMA_HID_KEY_COMMA;
			break;
		case SDLK_PERIOD:
			dest_ev->key = LIBAROMA_HID_KEY_DOT;
			break;
		case SDLK_SLASH:
			dest_ev->key = LIBAROMA_HID_KEY_SLASH;
			break;
		case SDLK_RSHIFT:
			dest_ev->key = LIBAROMA_HID_KEY_RIGHTSHIFT;
			break;
		case SDLK_LCTRL:
			dest_ev->key = LIBAROMA_HID_KEY_LEFTCTRL;
			break;
	#ifdef LIBAROMA_PLATFORM_SDL2
		case SDLK_LGUI:
	#else
		case SDLK_LMETA:
		case SDLK_LSUPER:
	#endif
			dest_ev->key = LIBAROMA_HID_KEY_LEFTMETA;
			break;
		case SDLK_LALT:
			dest_ev->key = LIBAROMA_HID_KEY_LEFTALT;
			break;
		case SDLK_SPACE:
			dest_ev->key = LIBAROMA_HID_KEY_SPACE;
			break;
	#ifndef LIBAROMA_PLATFORM_SDL2
		case SDLK_MODE: /* AltGr key in SDL 1.2 */
	#endif
		case SDLK_RALT:
			dest_ev->key = LIBAROMA_HID_KEY_RIGHTALT;
			break;
	#ifdef LIBAROMA_PLATFORM_SDL2
		case SDLK_RGUI:
	#else
		case SDLK_RMETA:
		case SDLK_RSUPER:
	#endif
			dest_ev->key = LIBAROMA_HID_KEY_RIGHTMETA;
			break;
		case SDLK_RCTRL:
			dest_ev->key = LIBAROMA_HID_KEY_RIGHTCTRL;
			break;
		/* now check navigation & edition keys section (arrows, insert, PrtScr...) */
		case SDLK_SYSREQ:
			dest_ev->key = LIBAROMA_HID_KEY_SYSRQ;
			break;
	#ifdef LIBAROMA_PLATFORM_SDL2
		case SDLK_SCROLLLOCK:
	#else
		case SDLK_SCROLLOCK:
	#endif
			dest_ev->key = LIBAROMA_HID_KEY_SCROLLLOCK;
			break;
		case SDLK_PAUSE:
			dest_ev->key = LIBAROMA_HID_KEY_PAUSE;
			break;
		case SDLK_INSERT:
			dest_ev->key = LIBAROMA_HID_KEY_INSERT;
			break;
		case SDLK_HOME:
			dest_ev->key = LIBAROMA_HID_KEY_HOME;
			break;
		case SDLK_PAGEUP:
			dest_ev->key = LIBAROMA_HID_KEY_PAGEUP;
			break;
		case SDLK_DELETE:
			dest_ev->key = LIBAROMA_HID_KEY_DELETE;
			break;
		case SDLK_END:
			dest_ev->key = LIBAROMA_HID_KEY_END;
			break;
		case SDLK_PAGEDOWN:
			dest_ev->key = LIBAROMA_HID_KEY_PAGEDOWN;
			break;
		case SDLK_LEFT:
			dest_ev->key = LIBAROMA_HID_KEY_LEFT;
			break;
		case SDLK_RIGHT:
			dest_ev->key = LIBAROMA_HID_KEY_RIGHT;
			break;
		/* finally, check keypad keys */
		/* thanks SDL for needlessly renaming some keys in the 2.0 version,
		 * very helpful for multi-version code & readability.
		 * LMETA/LSUPER to LGUI unifies Windows key, but all others make no sense. */
	#ifdef LIBAROMA_PLATFORM_SDL2
		case SDLK_NUMLOCKCLEAR:
	#else
		case SDLK_NUMLOCK:
	#endif
			dest_ev->key = LIBAROMA_HID_KEY_NUMLOCK;
			break;
		case SDLK_KP_DIVIDE:
			dest_ev->key = LIBAROMA_HID_KEY_KPSLASH;
			break;
		case SDLK_KP_MULTIPLY:
			dest_ev->key = LIBAROMA_HID_KEY_KPASTERISK;
			break;
		case SDLK_KP_MINUS:
			dest_ev->key = LIBAROMA_HID_KEY_KPMINUS;
			break;
	#ifdef LIBAROMA_PLATFORM_SDL2
		case SDLK_KP_7:
	#else
		case SDLK_KP7:
	#endif
			dest_ev->key = LIBAROMA_HID_KEY_KP7;
			break;
	#ifdef LIBAROMA_PLATFORM_SDL2
		case SDLK_KP_8:
	#else
		case SDLK_KP8:
	#endif
			dest_ev->key = LIBAROMA_HID_KEY_KP8;
			break;
	#ifdef LIBAROMA_PLATFORM_SDL2
		case SDLK_KP_9:
	#else
		case SDLK_KP9:
	#endif
			dest_ev->key = LIBAROMA_HID_KEY_KP9;
			break;
		case SDLK_KP_PLUS:
			dest_ev->key = LIBAROMA_HID_KEY_KPPLUS;
			break;
	#ifdef LIBAROMA_PLATFORM_SDL2
		case SDLK_KP_4:
	#else
		case SDLK_KP4:
	#endif
			dest_ev->key = LIBAROMA_HID_KEY_KP4;
			break;
	#ifdef LIBAROMA_PLATFORM_SDL2
		case SDLK_KP_5:
	#else
		case SDLK_KP5:
	#endif
			dest_ev->key = LIBAROMA_HID_KEY_KP5;
			break;
	#ifdef LIBAROMA_PLATFORM_SDL2
		case SDLK_KP_6:
	#else
		case SDLK_KP6:
	#endif
			dest_ev->key = LIBAROMA_HID_KEY_KP6;
			break;
	#ifdef LIBAROMA_PLATFORM_SDL2
		case SDLK_KP_1:
	#else
		case SDLK_KP1:
	#endif
			dest_ev->key = LIBAROMA_HID_KEY_KP1;
			break;
	#ifdef LIBAROMA_PLATFORM_SDL2
		case SDLK_KP_2:
	#else
		case SDLK_KP2:
	#endif
			dest_ev->key = LIBAROMA_HID_KEY_KP2;
			break;
	#ifdef LIBAROMA_PLATFORM_SDL2
		case SDLK_KP_3:
	#else
		case SDLK_KP3:
	#endif
			dest_ev->key = LIBAROMA_HID_KEY_KP3;
			break;
	#ifdef LIBAROMA_PLATFORM_SDL2
		case SDLK_KP_0:
	#else
		case SDLK_KP0:
	#endif
			dest_ev->key = LIBAROMA_HID_KEY_KP0;
			break;
		case SDLK_KP_PERIOD:
			dest_ev->key = LIBAROMA_HID_KEY_KPDOT;
			break;
		default:
			dest_ev->key = LIBAROMA_HID_KEY_UNKNOWN;
			break;
	}
	/* return set key */
	return LIBAROMA_HID_EV_RET_RAWKEY;
}

/*
 * function : libaroma init hid driver
 */
byte libaroma_hid_driver_init(LIBAROMA_HIDP me) {
	return SDLHIDRV_init(me);
}

#endif /* __libaroma_linux_hid_driver_c__ */
