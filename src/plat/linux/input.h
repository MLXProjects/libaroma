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
 * Filename		: hid_driver.h
 * Description : linux input hid driver
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 26/01/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_linux_hid_driver_h__
#define __libaroma_linux_hid_driver_h__
#include <aroma_internal.h>

/*
 * headers
 */
#include <linux/input.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>

/*
 * UNIVERSAL DEVICE - INPUT DRIVER
 *	 Using Linux Input Device for Android & Linux
 *	 Prefix : LINUXHIDRV_
 */

#ifdef __cplusplus
extern "C" {
#endif
/*
 * defines & macros
 */
#define LINUXHIDRV_DEVPATH "/dev/input"
#define LINUXHIDRV_BOARD_VKEY_PATH "/sys/board_properties/virtualkeys."
#define LINUXHIDRV_MAXDEV 0xf
#define LINUXHIDRV_SIZEOF_BIT_ARRAY(bits) ((bits + 7) / 8)
#define LINUXHIDRV_TEST_BIT(bit, array) (array[bit/8] & (1<<(bit%8)))

/*
 * enum : device type
 */
enum {
	LINUXHIDRV_DEVCLASS_KEYBOARD	= 0x01,
	LINUXHIDRV_DEVCLASS_TOUCH		= 0x02,
	LINUXHIDRV_DEVCLASS_MULTITOUCH	= 0x04,
	LINUXHIDRV_DEVCLASS_POINTER		= 0x08
};

/*
 * enum : position state
 */
enum {
	LINUXHIDRV_POS_ST_SYNC_X	= 0x01,
	LINUXHIDRV_POS_ST_SYNC_Y	= 0x02,
	LINUXHIDRV_POS_ST_DOWNED	= 0x04,
	LINUXHIDRV_POS_ST_LASTSYNC	= 0x08,
	LINUXHIDRV_POS_ST_RLS_NEXT	= 0x10,
	LINUXHIDRV_POS_ST_IS_VKEY	= 0x20
};

/*
 * structure : position
 */
typedef struct {
	int x;						/* last raw x event */
	int y;						/* last raw y event */
	int tx;						/* translated x position */
	int ty;						/* translated y position */
	int vk;						/* virtual key code id */
	byte state;					/* state */
	struct input_absinfo xi;	/* calibrate x */
	struct input_absinfo yi;	/* calibrate y */
} LINUXHIDRV_POS, *LINUXHIDRV_POSP;

/*
 * structure : virtual keys data
 */
typedef struct {
	int scan;	/* scan code */
	int x;		/* x */
	int y;		/* y */
	int w;		/* width */
	int h;		/* height */
} LINUXHIDRV_VK, *LINUXHIDRV_VKP;

/*
 * structure : device data
 */
typedef struct {
	/* Device Into */
	int id;							/* device id */
	byte devclass;					/* device class */
	char file[10];					/* input device filename */
	char name[64];					/* device name */
	byte down;						/* pressed */
	int vkn;						/* virtual key count */
	LINUXHIDRV_VKP	vks;			/* virtual keys */
	LINUXHIDRV_POS	p;				/* abs position */
} LINUXHIDRV_DEVICE, *LINUXHIDRV_DEVICEP;

/*
 * structure : internal driver data
 */
typedef struct {
	int n; /* number of devices*/
	/* pool */
	struct pollfd fds[LINUXHIDRV_MAXDEV];
	/* device data */
	LINUXHIDRV_DEVICE dev[LINUXHIDRV_MAXDEV]; /* Devices Data */
	/* configurations */
	byte touch_swap_xy;	/* swap x with y */
	byte touch_flip_x;	/* x was flipped */
	byte touch_flip_y;	/* y was flipped */
} LINUXHIDRV_INTERNAL, *LINUXHIDRV_INTERNALP;

extern byte LINUXHIDRV_translate_keyboard(LIBAROMA_HIDP me, LINUXHIDRV_DEVICEP dev,
                             LIBAROMA_HID_EVENTP dest_ev, struct input_event * ev);

extern byte LINUXHIDRV_translate_touch(
		LIBAROMA_HIDP me,
		LINUXHIDRV_DEVICEP dev,
		LIBAROMA_HID_EVENTP dest_ev,
		struct input_event * ev);

extern byte LINUXHIDRV_translate_pointer(LIBAROMA_HIDP me, LINUXHIDRV_DEVICEP dev,
									LIBAROMA_HID_EVENTP dest_ev, struct input_event * ev);

#ifdef __cplusplus
}
#endif
#endif /* __libaroma_linux_hid_driver_h__ */