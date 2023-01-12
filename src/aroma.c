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
 * Description : libaroma main initializer & runtime config
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 06/04/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_aroma_c__
#define __libaroma_aroma_c__
#include <aroma_internal.h>

#ifdef __linux__
#include <sys/wait.h>
#include <sys/stat.h>
#ifdef LIBAROMA_TTY_KDSETMODE
#include <linux/kd.h>
#include <sys/ioctl.h>
#endif /* LIBAROMA_TTY_KDSETMODE */
#endif /* __linux__ */

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
static byte _libaroma_init_status=0;

#ifdef LIBAROMA_FB_INITHELPER
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
} /* End of libaroma_config_cursor_path */

/*
 * Function		: libaroma_config_force_cursor
 * Return Value: void
 * Descriptions: force window manager to init & render cursor
 */
void libaroma_config_force_cursor(){
	libaroma_config()->wm_force_cursor=1;
} /* End of libaroma_config_force_cursor */

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
	char ttypath[32];
	snprintf(ttypath, 32, "/dev/tty%d", LIBAROMA_TTY_KDSETMODE);
	int tty = open(ttypath, O_RDWR);
	ioctl(tty, KDSETMODE, KD_GRAPHICS);
	close(tty);
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
	#ifdef LIBAROMA_FB_INITHELPER
	ALOGD("Starting LIBAROMA_FB_INITHELPER thread");
	if (libaroma_thread_create(&_init_helper, _libaroma_init_helper, NULL) != 0){
		ALOGE("libaroma_start cannot start init helper thread...");
		return 0;
	}
	while(!libaroma_msg_runstate()){ //wait until initialization is done in helper thread
		switch (_libaroma_init_status){
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
				//ALOGV("waiting for start finished #%d, init_status=%d, runstate=%d", i++, _libaroma_init_status, libaroma_msg_runstate());
				libaroma_sleep(50);
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

	_libaroma_init_status=1;

	return 1;
}

/*
 * Function		: libaroma_end
 * Return Value: byte
 * Descriptions: End libaroma application
 */
byte libaroma_end() {
	if (!_libaroma_init_status) {
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
	char ttypath[32];
	snprintf(ttypath, 32, "/dev/tty%d", LIBAROMA_TTY_KDSETMODE);
	int tty = open(ttypath, O_RDWR);
	ioctl(tty, KDSETMODE, KD_TEXT);
	close(tty);
#endif

	return 1;
}
#ifdef LIBAROMA_FB_INITHELPER
void *_libaroma_init_helper(void *cookie){
	if (libaroma_fb_init()) {
		if (libaroma_hid_init()) {
			if (!libaroma_msg_init()) {
				_libaroma_init_status=3;
			}
		}
		else _libaroma_init_status=2;
	}
	else _libaroma_init_status=1;
	return NULL;
}
#endif

/*
 * Variable		: _LIBAROMA_VERSION*
 * Type				: char []
 * Descriptions: version storage
 */
static char _LIBAROMA_VERSION[50] = {0};
static char _LIBAROMA_VERSION_FULLVER[50] = {0};
static char _LIBAROMA_VERSION_SIGNATURE[80] = {0};

/*
 * Function		: _libaroma_version
 * Return Value: char *
 * Descriptions: Get libaroma version string
 */
char * _libaroma_version() {
	if (_LIBAROMA_VERSION[0] == 0) {
		snprintf(_LIBAROMA_VERSION,
						 50,
						 "%i.%i.%i",
						 LIBAROMA_CONFIG_VERSION_MAJOR,
						 LIBAROMA_CONFIG_VERSION_MINOR,
						 LIBAROMA_CONFIG_VERSION_MICRO
						);
	}
	return _LIBAROMA_VERSION;
}

/*
 * Function		: _libaroma_version_fullver
 * Return Value: char *
 * Descriptions: Get libaroma full version string
 */
char * _libaroma_version_fullver() {
	if (_LIBAROMA_VERSION_FULLVER[0] == 0) {
		snprintf(_LIBAROMA_VERSION_FULLVER,
						 50,
						 "%s (%s;%s;%s)",
						 _libaroma_version(),
						 LIBAROMA_CONFIG_VERSION_BUILD,
						 LIBAROMA_CONFIG_OS,
						 LIBAROMA_CONFIG_CODENAME
						);
	}
	return _LIBAROMA_VERSION_FULLVER;
}

/*
 * Function		: _libaroma_version_signature
 * Return Value: char *
 * Descriptions: Get libaroma version signature string
 */
char * _libaroma_version_signature() {
	if (_LIBAROMA_VERSION_SIGNATURE[0] == 0) {
		snprintf(_LIBAROMA_VERSION_SIGNATURE,
						 80,
						 "%s Version %s",
						 LIBAROMA_CONFIG_NAME,
						 _libaroma_version_fullver()
						);
	}
	return _LIBAROMA_VERSION_SIGNATURE;
}

/*
 * Function		: libaroma_info
 * Return Value: char *
 * Descriptions: Get libaroma version information
 */
char * libaroma_info(int type) {
	switch (type) {
		case LIBAROMA_INFO_VERSION:
			return _libaroma_version();
			break;

		case LIBAROMA_INFO_OS:
			return LIBAROMA_CONFIG_OS;
			break;

		case LIBAROMA_INFO_AUTHOR:
			return LIBAROMA_CONFIG_AUTHOR;
			break;

		case LIBAROMA_INFO_CODENAME:
			return LIBAROMA_CONFIG_CODENAME;
			break;

		case LIBAROMA_INFO_BUILD:
			return LIBAROMA_CONFIG_VERSION_BUILD;
			break;

		case LIBAROMA_INFO_FULLVER:
			return _libaroma_version_fullver();
			break;

		case LIBAROMA_INFO_COPYRIGHT:
			return
				"Copyright (c) " LIBAROMA_CONFIG_YEAR " " LIBAROMA_CONFIG_AUTHOR;

		case LIBAROMA_INFO_SIGNATURE:
			return _libaroma_version_signature();
			break;
	}
	return LIBAROMA_CONFIG_NAME;
}

#ifdef LIBAROMA_RUNTIME_MONITOR
/*
 * Structure	 : _LIBAROMA_RUNTIME
 * Typedef		 : LIBAROMA_RUNTIME, * LIBAROMA_RUNTIMEP
 * Descriptions: runtime pid structure
 */
typedef struct _LIBAROMA_RUNTIME LIBAROMA_RUNTIME;
typedef struct _LIBAROMA_RUNTIME * LIBAROMA_RUNTIMEP;
struct _LIBAROMA_RUNTIME{
	pid_t parent;
	pid_t monitor;
	pid_t me;
	byte core_online[16];
	byte core_num;
};

/*
 * Variable		: _libaroma_runtime
 * Type				: LIBAROMA_RUNTIME
 * Descriptions: runtime pid storage
 */
static LIBAROMA_RUNTIME _libaroma_runtime = { 0 };
#endif /* LIBAROMA_RUNTIME_MONITOR */

/*
 * Function		: libaroma_runtime_activate_cores
 * Return Value: void
 * Descriptions: activate multicore
 */
void libaroma_runtime_activate_cores(int num_cores){
/* only for linux */
#ifdef LIBAROMA_RUNTIME_MONITOR
	int i;
	_libaroma_runtime.core_num=0;
	FILE * fp;
	char path[256];

	for (i=0;i<num_cores;i++){
		snprintf(path,256,"/sys/devices/system/cpu/cpu%i/online",i);
		struct stat st;
		if (stat(path, &st) < 0) {
			break;
		}
		/* read value */
		fp = fopen(path, "r");
		int is_online = 0;
		if(fp){
			is_online=(fgetc(fp)=='0')?1:0;
			fclose(fp);
		}
		else{
			break;
		}

		_libaroma_runtime.core_online[i]=is_online;
		fp = fopen(path, "w+");
		if(fp){
			fputc('1',fp);
			fclose(fp);
		}
		_libaroma_runtime.core_num++;
	}

	/* max power */
	char cmds[1024];
	for (i=0;i<_libaroma_runtime.core_num;i++){
		snprintf(path,256,"/sys/devices/system/cpu/cpu%i/cpufreq",i);
		snprintf(cmds,1024,"%s/scaling_governor",path);
		fp = fopen(cmds, "w+");
		if(fp){
			fwrite("performance",1,11,fp);
			fclose(fp);
		}
	}
	ALOGI("Processor Activated : %i Core(s)",_libaroma_runtime.core_num);
#endif
} /* End of libaroma_runtime_activate_cores */

/*
 * Function		: libaroma_runtime_rollback_cores
 * Return Value: void
 * Descriptions: rollback processor state
 */
void libaroma_runtime_rollback_cores(){
#ifdef LIBAROMA_RUNTIME_MONITOR
	int i;
	FILE * fp;
	char path[256];
	for (i=0;i<_libaroma_runtime.core_num;i++){
		snprintf(path,256,"/sys/devices/system/cpu/cpu%i/online",i);
		struct stat st;
		if (stat(path, &st) < 0) {
			break;
		}
		if(!_libaroma_runtime.core_online[i]){
			fp = fopen(path, "w+");
			if (fp){
				fputc('0',fp);
				fclose(fp);
			}
		}
	}
	_libaroma_runtime.core_num=0;
#endif
} /* End of libaroma_runtime_rollback_cores */

/*
 * Function		: libaroma_runtime_mute_parent
 * Return Value: void
 * Descriptions: Pause parent process
 */
void libaroma_runtime_mute_parent() {
#ifdef LIBAROMA_RUNTIME_MONITOR
	kill(_libaroma_runtime.parent, 19);
#endif
} /* End of libaroma_runtime_mute_parent */


/*
 * Function		: libaroma_runtime_continue_parent
 * Return Value: void
 * Descriptions: Continue parent process
 */
void libaroma_runtime_continue_parent() {
#ifdef LIBAROMA_RUNTIME_MONITOR
	kill(_libaroma_runtime.parent, 18);
#endif
} /* End of libaroma_runtime_continue_parent */

/*
 * Function		: libaroma_runtime_init
 * Return Value: void
 * Descriptions: Init libaroma runtime
 */
void libaroma_runtime_init() {
#ifdef LIBAROMA_RUNTIME_MONITOR
	_libaroma_runtime.parent	 = getppid();	/* root */
	_libaroma_runtime.monitor	= getpid();	 /* monitor */
	_libaroma_runtime.me			 = fork();		 /* child - fork */

	if (_libaroma_runtime.me == 0) {
		_libaroma_runtime.me = getpid();
		ALOGS("RUNTIME: Application @%i",_libaroma_runtime.me);
		return; /* start application */
	}
	else if (_libaroma_runtime.me < 0) {
		ALOGE("RUNTIME: Cannot Start Application Instance...");
		ALOGV("RUNTIME: Make sure to continue parent(%i)...",
				_libaroma_runtime.parent);
		libaroma_runtime_continue_parent();
		ALOGI("RUNTIME: Halting...");
		_exit(-1); /* error */
	}

	ALOGS("RUNTIME: MONITORING(R:%i, M:%i, A:%i)",
			_libaroma_runtime.parent,
			_libaroma_runtime.monitor,
			_libaroma_runtime.me
			);

	int status;
	waitpid(_libaroma_runtime.me, &status, 0);
	ALOGV("RUNTIME: Make sure to continue parent(%i)...",
			_libaroma_runtime.parent);
	libaroma_runtime_continue_parent();
	ALOGS("RUNTIME: Exit Status (%i)", status);
	_exit(0);
#endif
} /* End of libaroma_runtime_init */

#ifdef __cplusplus
}
#endif

#endif /* __libaroma_aroma_c__ */

