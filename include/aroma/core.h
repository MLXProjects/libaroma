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
 * Filename		: core.h
 * Description : libaroma core headers
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 19/01/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_aroma_h__
	#error "Include <aroma.h> instead."
#endif
#ifndef __libaroma_core_h__
#define __libaroma_core_h__

/*
 * typedef : Window & Control Structure
 */
typedef struct _LIBAROMA_WINDOW LIBAROMA_WINDOW;
typedef struct _LIBAROMA_WINDOW * LIBAROMA_WINDOWP;
typedef struct _LIBAROMA_CONTROL LIBAROMA_CONTROL;
typedef struct _LIBAROMA_CONTROL * LIBAROMA_CONTROLP;

/* utils */
#include "utils_array.h"		/* array */
#include "utils_time.h"			/* time */
#include "utils_zip.h"			/* zip */
#include "utils_string.h"		/* strings */
#include "utils_utf8.h"			/* utf8 */
#include "utils_stream.h"		/* stream */
#include "utils_json.h"			/* json */
#include "utils_sys.h"			/* system */
#include "utils_motions.h"		/* motion interpolation */
#include "utils_xml.h"			/* xml */

/* graphics engine*/
#include "graph_engine.h"		/* graph engine */
#include "graph_cv.h"			/* canvas */
#include "graph_fb.h"			/* framebuffer */
#include "graph_draw.h"			/* common drawing */
#include "graph_jpeg.h"			/* jpeg */
#include "graph_png.h"			/* png */
#include "graph_svg.h"			/* svg */
#include "graph_img.h"			/* image */
#include "graph_text.h"			/* font & text */
#include "graph_art.h"			/* art drawing */

/* input system */
#include "hid.h"				/* hid & input handler */
#include "hid_keys.h"			/* input key codes */
#include "hid_msg.h"			/* message queue */

/* ui modules */
#include "ui_colormgr.h"		/* color manager */
#include "ui_ctl.h"				/* controls */
#include "ui_dlg.h"				/* dialogs */
#include "ui_win.h"				/* window */
#include "ui_winmgr.h"			/* window manager */

/* control set */
#include "ctl_bar.h"
#include "ctl_label.h"
#include "ctl_prog.h"
#include "ctl_btn.h"
#include "ctl_pager.h"
#include "ctl_tabs.h"
#include "ctl_scroll.h"
#include "ctl_slider.h"
#include "ctl_list.h"
#include "ctl_frag.h"
#include "ctl_img.h"
#include "ctl_clock.h"

/* list items */
#include "list_caption.h"
#include "list_check.h"
#include "list_div.h"
#include "list_img.h"
#include "list_menu.h"
#include "list_option.h"
#include "list_text.h"

/*  */
typedef word (*color_handler)(const char *color_str);

/*
 * typedef : Libaroma configuration
 */
typedef struct{
	char fb_shm_name[256];				/* framebuffer shmem file name */
	byte snapshoot_fb;					/* try to get screen contents before start */
	byte multicore_init_num;			/* max cpu cores to init at startup (default: 8) */
	color_handler custom_color_handler;	/* color translator callback for text tags parser */
	byte runtime_monitor;				/* enable app runtime monitor (to handle segfaults and such) */
	int sdl_wm_width;					/* startup width for SDL window */
	int sdl_wm_height;					/* startup height for SDL window */
	char *sdl_wm_title;					/* sdl window title */
	byte gfx_override_rgb;				/* use gfx_default_rgb for framebuffer initialization */
	int gfx_override_dpi;				/* override dpi at framebuffer initialization */
	byte gfx_default_rgb[3];			/* display default RGB pixel order */
	char *wm_cursor_res;				/* path to cursor image, used if mice connected */
	byte wm_force_cursor;				/* force cursor init at window manager */
	byte sdl_mousemove;					/* send mouse move events while buttons released */
	byte sdl_nocursor;					/* toggle SDL window cursor at startup */

} LIBAROMA_CONFIG, * LIBAROMA_CONFIGP;

/*
 * Function		: libaroma_config
 * Return Value: LIBAROMA_CONFIGP
 * Descriptions: get runtime config
 */
LIBAROMA_CONFIGP libaroma_config();

/*
 * Function		: libaroma_sdl_startup_size
 * Return Value: void
 * Descriptions: set startup width/height for SDL window
 */
void libaroma_sdl_startup_size(int width, int height);

/*
 * Function		: libaroma_sdl_window_title
 * Return Value: void
 * Descriptions: set startup title for SDL window
 */
void libaroma_sdl_window_title(char *title);

/*
 * Function		: libaroma_gfx_override_rgb
 * Return Value: void
 * Descriptions: override rgb order when initializing framebuffer
 */
void libaroma_gfx_override_rgb(byte override, byte r, byte g, byte b);

/*
 * Function		: libaroma_gfx_startup_dpi
 * Return Value: void
 * Descriptions: override dpi when initializing framebuffer
 */
byte libaroma_gfx_startup_dpi(int dpi);

/*
 * Function		: libaroma_config_cursor_path
 * Return Value: void
 * Descriptions: set wm cursor image path
 */
void libaroma_config_cursor_path(char *uri);

/*
 * Function		: libaroma_config_force_cursor
 * Return Value: void
 * Descriptions: force window manager to init & render cursor
 */
void libaroma_config_force_cursor();

/*
 * Function		: libaroma_info
 * Return Value: char *
 * Descriptions: Get libaroma version information
 */
char * libaroma_info(int type);

/*
 * Function		: libaroma_debug_set_output
 * Return Value: void
 * Descriptions: set debug output fd
 */
void libaroma_debug_set_output(FILE * fd);

/*
 * Function		: libaroma_debug_set_tag
 * Return Value: void
 * Descriptions: set debug tag
 */
void libaroma_debug_set_tag(char * tag);

/*
 * Function		: libaroma_debug_set_prefix
 * Return Value: void
 * Descriptions: set debug prefix
 */
void libaroma_debug_set_prefix(char * prefix);

/*
 * Function		: libaroma_start
 * Return Value: byte
 * Descriptions: Start libaroma application
 */
byte libaroma_start();

/*
 * Function		: libaroma_end
 * Return Value: byte
 * Descriptions: End libaroma application
 */
byte libaroma_end();

#endif /* __libaroma_core_h__ */
