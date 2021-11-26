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
 * Filename		: aroma.c
 * Description : libaroma main runtime config & initializer
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 06/04/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_aroma_c__
#define __libaroma_aroma_c__
#ifndef __libaroma_internal_init_release
#define __libaroma_internal_init_release
#endif
#include <aroma_internal.h>

#ifdef LIBAROMA_TTY_KDSETMODE
	#include <linux/kd.h>
	#include <sys/ioctl.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

/* init & release function */
byte libaroma_hid_init();
void libaroma_hid_release();
byte libaroma_fb_init();
byte libaroma_fb_release();
byte libaroma_msg_init();
void libaroma_msg_release();
byte libaroma_wm_init();
byte libaroma_wm_release();
void libaroma_runtime_init();
void libaroma_runtime_activate_cores(int num_cores);
void libaroma_runtime_mute_parent();
void libaroma_runtime_rollback_cores();
byte libaroma_lang_init();
byte libaroma_lang_release();
byte libaroma_timer_init();
byte libaroma_timer_release();
byte libaroma_font_init();
byte libaroma_font_release();
static byte _init_status=0;
#ifdef LIBAROMA_INIT_HELPER
extern byte libaroma_msg_runstate();
static LIBAROMA_THREAD _init_helper;
void *_libaroma_init_helper(void *cookie);
#endif

/*
 * Variable		: _libaroma_config
 * Type				: LIBAROMA_CONFIG
 * Descriptions: runtime configuration
 */
static LIBAROMA_CONFIG _libaroma_config={0};
static byte _libaroma_config_ready=0;
static FILE * _libaroma_debug_fp=NULL;
static char _libaroma_debug_tag[256]="LIBAROMA()";
static char _libaroma_debug_prefix[32]="";


/*
 * Function		: _libaroma_config_default
 * Return Value: void
 * Descriptions: set default runtime configuration
 */
void _libaroma_config_default() {
	
#ifdef LIBAROMA_FB_SHMEM_NAME
	snprintf(_libaroma_config.fb_shm_name,256,"%s",LIBAROMA_FB_SHMEM_NAME);
#else
	_libaroma_config.fb_shm_name[0]=0;
#endif
	if (!_libaroma_debug_fp)
		_libaroma_debug_fp=stdout; /* default pipe */
	_libaroma_config.snapshoot_fb = 1; /* snapshoot after graph init */
	if (_libaroma_config.sdl_wm_width < 1) _libaroma_config.sdl_wm_width=360;
	if (_libaroma_config.sdl_wm_height < 1) _libaroma_config.sdl_wm_height=600;
	if (_libaroma_config.sdl_wm_title==NULL) _libaroma_config.sdl_wm_title="Libaroma";
	_libaroma_config_ready = 1;
} /* End of libaroma_config_default */

/*
 * Function		: libaroma_startup_size
 * Return Value: void
 * Descriptions: set startup width/height for SDL window
 */
void libaroma_sdl_startup_size(int width, int height){
	_libaroma_config.sdl_wm_width=width;
	_libaroma_config.sdl_wm_height=height;
}

/*
 * Function		: libaroma_sdl_window_title
 * Return Value: void
 * Descriptions: set startup title for SDL window
 */
void libaroma_sdl_window_title(char *title){
	_libaroma_config.sdl_wm_title=title;
}

/*
 * Function		: libaroma_sdl_startup_size
 * Return Value: void
 * Descriptions: override rgb order when initializing framebuffer
 */
void libaroma_gfx_override_rgb(byte override, byte r, byte g, byte b){
	_libaroma_config.gfx_override_rgb=override;
	_libaroma_config.gfx_default_rgb[0]=r;
	_libaroma_config.gfx_default_rgb[1]=g;
	_libaroma_config.gfx_default_rgb[2]=b;
}

byte libaroma_gfx_startup_dpi(int dpi){
	if ((dpi >= 100)&&(dpi <= 960)){
		libaroma_config()->gfx_override_dpi = dpi;
		return 1;
	}
	else {
		ALOGW("libaroma_gfx_startup_dpi invalid override dpi=%d", dpi);
		return 0;
	}
}

/*
 * Function		: libaroma_config_cursor_path
 * Return Value: void
 * Descriptions: set wm cursor image path
 */
void libaroma_config_cursor_path(char *uri){
	libaroma_config()->wm_cursor_res = uri;
}

/*
 * Function		: libaroma_debug_output
 * Return Value: FILE *
 * Descriptions: get debug output fd
 */
FILE * libaroma_debug_output(){
	if (!_libaroma_debug_fp){
		_libaroma_debug_fp=stdout;
	}
	return _libaroma_debug_fp;
} /* End of libaroma_debug_output */

/*
 * Function		: libaroma_debug_tag
 * Return Value: char *
 * Descriptions: get debug tag
 */
char * libaroma_debug_tag(){
	return _libaroma_debug_tag;
} /* End of libaroma_debug_tag */

/*
 * Function		: libaroma_debug_prefix
 * Return Value: char *
 * Descriptions: get debug prefix
 */
char * libaroma_debug_prefix(){
	return _libaroma_debug_prefix;
} /* End of libaroma_debug_prefix */

/*
 * Function		: libaroma_debug_set_tag
 * Return Value: void
 * Descriptions: set debug tag
 */
void libaroma_debug_set_tag(char * tag){
	if (tag){
		snprintf(_libaroma_debug_tag,256,"%s",tag);
	}
} /* End of libaroma_debug_set_tag */

/*
 * Function		: libaroma_debug_set_prefix
 * Return Value: void
 * Descriptions: set debug prefix
 */
void libaroma_debug_set_prefix(char * prefix){
	if (prefix){
		snprintf(_libaroma_debug_prefix,32,"%s",prefix);
	}
} /* End of libaroma_debug_set_prefix */

/*
 * Function		: libaroma_debug_set_output
 * Return Value: void
 * Descriptions: set debug output fd
 */
void libaroma_debug_set_output(FILE * fd){
	_libaroma_debug_fp = fd;
	if (!_libaroma_debug_fp){
		_libaroma_debug_fp=stdout;
	}
} /* End of libaroma_debug_set_output */

/*
 * Function		: libaroma_config
 * Return Value: LIBAROMA_CONFIGP
 * Descriptions: get runtime config
 */
LIBAROMA_CONFIGP libaroma_config(){
	if (!_libaroma_config_ready){
		_libaroma_config_default();
	}
	return &_libaroma_config;
} /* End of libaroma_config */

/*
 * Function		: libaroma_start
 * Return Value: byte
 * Descriptions: Start libaroma application
 */
byte libaroma_start() {

	srand(time(0));
	/* Welcome Message */
	ALOGI(
		"%s Version %s",
		libaroma_info(LIBAROMA_INFO_NAME),
		libaroma_info(LIBAROMA_INFO_FULLVER)
	);
	ALOGI("	%s", libaroma_info(LIBAROMA_INFO_COPYRIGHT));
	ALOGI(" ");

#ifdef LIBAROMA_TTY_KDSETMODE
	ALOGI("KDSETMODE = KD_GRAPHICS");
	int tty1 = open("/dev/tty1", O_RDWR);
	ioctl(tty1, KDSETMODE, KD_GRAPHICS);
#endif

	/* Init Safe Process Monitoring */
	if (libaroma_config()->runtime_monitor) {
		libaroma_runtime_init();
	}

#ifdef LIBAROMA_CONFIG_DEBUG_MEMORY
#if LIBAROMA_CONFIG_DEBUG_MEMORY >=1
	ALOGV("STARTING MEMORY TRACKING");
	___mtrack_init_free(0);
#endif
#endif

	/* Mute Parent */
	if (libaroma_config()->runtime_monitor == LIBAROMA_START_MUTEPARENT) {
		libaroma_runtime_mute_parent();
	}

	ALOGI("===================================================");

	if (libaroma_config()->multicore_init_num>0){
		/* activate processor/cores */
		libaroma_runtime_activate_cores(libaroma_config()->multicore_init_num);
	}
	#ifdef LIBAROMA_INIT_HELPER
	ALOGD("Starting LIBAROMA_INIT_HELPER thread");
	if (libaroma_thread_create(&_init_helper, _libaroma_init_helper, NULL) != 0){
		ALOGE("libaroma_start cannot start init helper thread...");
		return 0;
	}
	int i=0;
	while(!libaroma_msg_runstate()){ //wait until initialization is done in helper thread
		switch (_init_status){
			case 3:
				ALOGE("libaroma_start cannot start framebuffer...");
				return 0;
			case 2:
				ALOGE("libaroma_start cannot start hid engine...");
				return 0;
			case 1:
				ALOGE("libaroma_start cannot start message queue...");
				return 0;
			default:
				ALOGD("waiting for start finished #%d, init_status=%d, runstate=%d", i++, _init_status, libaroma_msg_runstate());
				libaroma_sleep(100);
				break;
		}
	}
	#else
	if (!libaroma_fb_init()) {
		ALOGE("libaroma_start cannot start framebuffer...");
		return 0;
	}
	
	if (!libaroma_hid_init()) {
		ALOGE("libaroma_start cannot start hid engine...");
		return 0;
	}

	if (!libaroma_msg_init()) {
		ALOGE("libaroma_start cannot start message queue...");
		return 0;
	}
	#endif
	
	if (!libaroma_font_init()) {
		ALOGE("libaroma_start cannot start font engine...");
		return 0;
	}

	if (!libaroma_lang_init()) {
		ALOGE("libaroma_start cannot start language engine...");
		return 0;
	}

	if (!libaroma_timer_init()) {
		ALOGE("libaroma_start cannot start timer engine...");
		return 0;
	}

	if (!libaroma_wm_init()){
		ALOGE("libaroma_start cannot start window manager...");
		return 0;
	}

	ALOGI("___________________________________________________");
	ALOGI(" ");
	
	_init_status=1;

	return 1;
}

/*
 * Function		: libaroma_end
 * Return Value: byte
 * Descriptions: End libaroma application
 */
byte libaroma_end() {
	if (!_init_status) {
		ALOGW("libaroma_end libaroma uninitialized");
		return 0;
	}
	
	ALOGI("___________________________________________________");
	ALOGI(" ");

	/* Release Engines */
	libaroma_wm_release();
	libaroma_timer_release();
	libaroma_lang_release();
	libaroma_msg_release();
	libaroma_hid_release();
	libaroma_font_release();
	libaroma_fb_release();
	libaroma_runtime_rollback_cores();

	ALOGI("===================================================");
#ifdef LIBAROMA_CONFIG_DEBUG_MEMORY
#if LIBAROMA_CONFIG_DEBUG_MEMORY >=1
	ALOGV("DUMPING MEMORY TRACKING");
	___mtrack_dump_leak();
	___mtrack_init_free(1);
#endif
#endif

#ifdef LIBAROMA_TTY_KDSETMODE
	ALOGI("KDSETMODE = KD_TEXT");
	int tty1 = open("/dev/tty1", O_RDWR);
	ioctl(tty1, KDSETMODE, KD_TEXT);
#endif

	return 1;
}
#ifdef LIBAROMA_INIT_HELPER
void *_libaroma_init_helper(void *cookie){
	if (!libaroma_fb_init()) {
		ALOGE("libaroma_init_helper cannot start framebuffer...");
		_init_status=1;
		return;
	}
	
	if (!libaroma_hid_init()) {
		ALOGE("libaroma_init_helper cannot start hid engine...");
		_init_status=2;
		return;
	}

	if (!libaroma_msg_init()) {
		ALOGE("libaroma_init_helper cannot start message queue...");
		_init_status=3;
		return;
	}
}
#endif

#ifdef __cplusplus
}
#endif

#endif /* __libaroma_aroma_c__ */

