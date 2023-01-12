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
#include "input.h"

/*
 * UNIVERSAL DEVICE - INPUT DRIVER
 *	 Using Linux Input Device for Android & Linux
 *	 Prefix : LINUXHIDRV_
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * forward functions
 */
void LINUXHIDRV_release(
		LIBAROMA_HIDP me);
byte LINUXHIDRV_getinput(
		LIBAROMA_HIDP me,
		LIBAROMA_HID_EVENTP dest_ev);
byte LINUXHIDRV_getinput_nodev(
		LIBAROMA_HIDP me,
		LIBAROMA_HID_EVENTP dest_ev);
byte LINUXHIDRV_init_device(
		LINUXHIDRV_INTERNALP mi,
		int fd,
		LINUXHIDRV_DEVICEP dev);

/*
 * function : check blacklisted devices
 */
byte LINUXHIDRV_blacklist(
		char * name) {
	/* no blacklisted */
	return 0;
}

/*
 * function : dump device info
 */
void LINUXHIDRV_dumpdev(
		LINUXHIDRV_DEVICEP dev) {
	/* print logs */
	ALOGI("INDR Input Device: %s (%s) - Class : %x",
		dev->name, dev->file, dev->devclass
	);
	ALOGV("	VKN : %d, CALIB : (%d,%d,%d,%d)", dev->vkn,
		dev->p.xi.minimum,
		dev->p.xi.maximum,
		dev->p.yi.minimum,
		dev->p.yi.maximum
	);
}

/*
 * function : init input device
 */
byte LINUXHIDRV_init(
		LIBAROMA_HIDP me) {
	/* allocating internal data */
	LINUXHIDRV_INTERNALP mi = (LINUXHIDRV_INTERNALP)
		calloc(sizeof(LINUXHIDRV_INTERNAL),1);

	/* set internal address */
	me->internal = (voidp) mi;
	/* set initial value */
	mi->n = 0;
	/* open input device directory */
	DIR * dir = opendir(LINUXHIDRV_DEVPATH);
	if (dir != 0) {
		struct dirent * de; /* dirent */
		int fd; /* temporary device fd */

		/* read input device directory */
		while ((de = readdir(dir))) {
			/* continue if filename not contain "event" */
			if (strncmp(de->d_name, "event", 5)) {
				continue;
			}
			/* open file handler */
			fd = openat(dirfd(dir), de->d_name, O_RDONLY);
			/* continue if openat failed */
			if (fd < 0) {
				continue;
			}
			/* cleanup device data */
			memset(&mi->dev[mi->n], 0, sizeof(LINUXHIDRV_DEVICE));
			/* set device id */
			mi->dev[mi->n].id = mi->n;
			/* set device filename */
			snprintf(mi->dev[mi->n].file, 10, "%s", de->d_name) < 0 ? abort() : (void)0 ;

			/* load virtualkeys if there are any */
			if (LINUXHIDRV_init_device(mi, fd, &mi->dev[mi->n])) {
				/* dump device information */
				LINUXHIDRV_dumpdev(&mi->dev[mi->n]);
				/* set pooling data and monitor it */
				mi->fds[mi->n].fd		 = fd;
				mi->fds[mi->n].events = POLLIN;
				/* if pointer connected, enable wm cursor drawing */
				if ((mi->dev[mi->n].devclass & LINUXHIDRV_DEVCLASS_POINTER)){
					me->has_mice=1;
				}
				/* increment the polling count */
				mi->n++;
			}
			else {
				/* dump device information */
				LINUXHIDRV_dumpdev(&mi->dev[mi->n]);
				/* cleanup device data */
				memset(&mi->dev[mi->n], 0, sizeof(LINUXHIDRV_DEVICE));
				/* don't monitor this device */
				close(fd);
				/* ignore it */
			}
			/* break when maximum device */
			if (mi->n == LINUXHIDRV_MAXDEV) {
				break;
			}
		}
		/* close dir */
		closedir(dir);

		/* set driver callbacks */
		me->release	= &LINUXHIDRV_release;
		if (mi->n == 0) {
			/* input device not found */
			ALOGW("LINUXHIDRV Input Device not found, make sure your program can receive events in another way");
			me->getinput = &LINUXHIDRV_getinput_nodev;
		}
		else {
			me->getinput= &LINUXHIDRV_getinput;
		}
		/* ok */
		return 1;
	}

	/* free internal data */
	free(mi);
	ALOGE("INDR ERROR: Can't access /dev/input...");
	/* error */
	return 0;
}

/*
 * function : release input driver instance
 */
void LINUXHIDRV_release(
		LIBAROMA_HIDP me) {
	/* is input instance initialized ? */
	if (me == NULL) {
		return;
	}
	/* get internal data */
	LINUXHIDRV_INTERNALP mi = (LINUXHIDRV_INTERNALP)
											me->internal;
	/* release devices data */
	while (mi->n-- > 0) {
		/* release virtual keys */
		if (mi->dev[mi->n].vkn) {
			free(mi->dev[mi->n].vks);
			mi->dev[mi->n].vkn = 0;
		}
		/* close fd */
		close(mi->fds[mi->n].fd);
	}
	/* free internal data */
	free(me->internal);
	me->internal = NULL;
}

/*
 * function : returns empty tokens
 */
static char * LINUXHIDRV_strtok_r(
		char * str,
		const char * delim,
		char ** save_str){
	if (!str) {
		if (!*save_str) {
			return NULL;
		}
		str = (*save_str) + 1;
	}
	*save_str = strpbrk(str, delim);
	if (*save_str) {
		**save_str = '\0';
	}
	return str;
}

/*
 * function : check non zero
 */
static byte LINUXHIDRV_nonzero(
		const bytep array_s,
		dword startIndex,
		dword endIndex) {
	const bytep end	 = array_s + endIndex;
	bytep array = array_s + startIndex;
	while (array != end) {
		if (*(array++) != 0) {
			return 1;
		}
	}
	return 0;
}

/*
 * function : get device type
 */
byte LINUXHIDRV_getdevclass(
		int fd) {
	/* figure out the kinds of events the device reports. */
	byte keyBitmask[(KEY_MAX + 1) / 8];
	byte absBitmask[(ABS_MAX + 1) / 8];
	byte relBitmask[(REL_MAX + 1) / 8];

	ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keyBitmask)), keyBitmask);
	ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absBitmask)), absBitmask);
	ioctl(fd, EVIOCGBIT(EV_REL, sizeof(relBitmask)), relBitmask);

	/* reset return value */
	byte ret = 0;

	/* check keyboard */
	byte haveKeyboardKeys =
		LINUXHIDRV_nonzero(keyBitmask, 0,
								 LINUXHIDRV_SIZEOF_BIT_ARRAY(BTN_MISC)) ||
		LINUXHIDRV_nonzero(keyBitmask,
								 LINUXHIDRV_SIZEOF_BIT_ARRAY(KEY_OK),
								 LINUXHIDRV_SIZEOF_BIT_ARRAY(KEY_MAX + 1));

	/* check gamepad */
	byte haveGamepadButtons =
		LINUXHIDRV_nonzero(keyBitmask,
								 LINUXHIDRV_SIZEOF_BIT_ARRAY(BTN_MISC),
								 LINUXHIDRV_SIZEOF_BIT_ARRAY(BTN_MOUSE)) ||
		LINUXHIDRV_nonzero(keyBitmask,
								 LINUXHIDRV_SIZEOF_BIT_ARRAY(BTN_JOYSTICK),
								 LINUXHIDRV_SIZEOF_BIT_ARRAY(BTN_DIGI));

	if (haveKeyboardKeys) {
		ret |= LINUXHIDRV_DEVCLASS_KEYBOARD;
	}

	/* check touch screen */
	if (LINUXHIDRV_TEST_BIT(ABS_MT_POSITION_X, absBitmask) &&
			LINUXHIDRV_TEST_BIT(ABS_MT_POSITION_Y, absBitmask)) {
		/* multitouch */
		if (LINUXHIDRV_TEST_BIT(BTN_TOUCH, keyBitmask) ||
				!haveGamepadButtons) {
			ret |= LINUXHIDRV_DEVCLASS_TOUCH;
			ret |= LINUXHIDRV_DEVCLASS_MULTITOUCH;
		}
	}
	else if (LINUXHIDRV_TEST_BIT(BTN_TOUCH, keyBitmask) &&
					 LINUXHIDRV_TEST_BIT(ABS_X, absBitmask) &&
					 LINUXHIDRV_TEST_BIT(ABS_Y, absBitmask)) {
		/* single touch */
		ret |= LINUXHIDRV_DEVCLASS_TOUCH;
	}

	/* mouse or gamepad */
	if (LINUXHIDRV_TEST_BIT(REL_X, relBitmask) &&
			LINUXHIDRV_TEST_BIT(REL_Y, relBitmask) &&
			/* TODO: mouse doesn't have Z axis, what about joystick? */
			!(LINUXHIDRV_TEST_BIT(REL_Z, relBitmask))) {
		ret |= LINUXHIDRV_DEVCLASS_POINTER;
	}

	return ret;
}

/*
 * function : init device
 */
byte LINUXHIDRV_init_device(
		LINUXHIDRV_INTERNALP mi,
		int fd,
		LINUXHIDRV_DEVICEP dev) {
	/* virtual key path */
	char		vk_path[PATH_MAX] = LINUXHIDRV_BOARD_VKEY_PATH;
	char	* ts = NULL;
	char		vks[2048];

	/* get device name */
	ssize_t len = ioctl(fd, EVIOCGNAME(sizeof(dev->name)), dev->name);
	if (len <= 0) {
		ALOGW("INDR ERROR: EVIOCGNAME for %d", dev->id);
		return 0;
	}

	/* blacklisted devices */
	if (LINUXHIDRV_blacklist(dev->name)) {
		return 0;
	}

	/* get device class */
	dev->devclass = LINUXHIDRV_getdevclass(fd);

	/* if class is none, ignore it */
	if (!dev->devclass) {
		return 0;
	}

	/* reset all values */
	memset(&dev->p, 0, sizeof(LINUXHIDRV_POS));
	dev->p.tx			 = -1;
	dev->p.ty			 = -1;
	dev->p.x				= -1;
	dev->p.y				= -1;
	dev->p.vk			 = -1;
	dev->vkn				= 0;
	dev->down			 = 0;

	/* if touchscreen, get calibration data & virtualkeys */
	if ((dev->devclass & LINUXHIDRV_DEVCLASS_TOUCH)) {
		/* calibration */
		if (dev->devclass & LINUXHIDRV_DEVCLASS_MULTITOUCH) {
			/* get multitouch calibrations data */
			ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), &dev->p.xi);
			ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), &dev->p.yi);
		}
		else {
			/* get singletouch calibrations data */
			ioctl(fd, EVIOCGABS(ABS_X), &dev->p.xi);
			ioctl(fd, EVIOCGABS(ABS_Y), &dev->p.yi);
		}

		/* virtualkeys.{device_name} */
		strcat(vk_path, dev->name);
		/* some devices split the keys from the touchscreen */
		int vk_fd = open(vk_path, O_RDONLY);

		if (vk_fd >= 0) {
			/* read contents */
			len = read(vk_fd, vks, sizeof(vks) - 1);
			close(vk_fd);

			/* return false on failed */
			if (len > 0) {
				/* add string break */
				vks[len] = 0;
				/* parse a line like:
				 * keytype:keycode:centerx:centery:width:height:keytype2:...
				 */
				for (ts = vks, dev->vkn = 1; *ts; ++ts) {
					if (*ts == ':') {
						dev->vkn++;
					}
				}
				dev->vkn /= 6;
				if (dev->vkn <= 0) {
					dev->vkn = 0;
				}
			}
		}

		/* allocate virtualkeys count */
		if (dev->vkn > 0) {
			dev->vks = malloc(sizeof(LINUXHIDRV_VK) * dev->vkn);
			int i;
			for (i = 0; i < dev->vkn; i++) {
				char * token[6];
				int j;
				for (j = 0; j < 6; j++) {
					token[j] = LINUXHIDRV_strtok_r((i || j) ? NULL : vks, ":", &ts);
				}
				if (strcmp(token[0], "0x01") != 0) {
					continue;
				}
				/* save it */
				dev->vks[i].scan	= strtol(token[1], NULL, 0);
				dev->vks[i].x		 = strtol(token[2], NULL, 0);
				dev->vks[i].y		 = strtol(token[3], NULL, 0);
				dev->vks[i].w		 = strtol(token[4], NULL, 0);
				dev->vks[i].h		 = strtol(token[5], NULL, 0);
				ALOGV("	VIRTUALKEY[%s,%i] (%i,%i,%i,%i,%i)",
							dev->file,
							i,
							dev->vks[i].scan,
							dev->vks[i].x,
							dev->vks[i].y,
							dev->vks[i].w,
							dev->vks[i].h
						 );
			}
		}
	}

	/* ok */
	return 1;
}

/*
 * function : translate raw data
 */
byte LINUXHIDRV_translate(LIBAROMA_HIDP me, LINUXHIDRV_DEVICEP dev,
										LIBAROMA_HID_EVENTP dest_ev, struct input_event * ev) {
	if (dev->devclass & LINUXHIDRV_DEVCLASS_TOUCH) {
		/* it's touch device - input_translate/translate_touch.c */
		return LINUXHIDRV_translate_touch(me, dev, dest_ev, ev);
	}
	else if (dev->devclass & LINUXHIDRV_DEVCLASS_POINTER) {
		/* it's pointer/relative device - input_translate/translate_mice.c */
		return LINUXHIDRV_translate_pointer(me, dev, dest_ev, ev);
	}
	else if (dev->devclass & LINUXHIDRV_DEVCLASS_KEYBOARD) {
		/* it's key device - input_translate/translate_key.c */
		return LINUXHIDRV_translate_keyboard(me, dev, dest_ev, ev);
	}

	/* don't process it */
	return LIBAROMA_HID_EV_RET_NONE;
}

/*
 * function : get input callback
 */
byte LINUXHIDRV_getinput(
		LIBAROMA_HIDP me,
		LIBAROMA_HID_EVENTP dest_ev) {
	/* get internal data */
	LINUXHIDRV_INTERNALP mi = (LINUXHIDRV_INTERNALP) me->internal;

	/* polling loop */
	do {
		int r = poll(mi->fds, mi->n, -1);
		if (me->internal == NULL) {
			/* if released */
			break;
		}
		else if (r > 0) {
			/* events loop */
			int n;
			for (n = 0; n < mi->n; n++) {
				if (mi->fds[n].revents & POLLIN) {
					/* read data */
					struct input_event ev;
					r = read(mi->fds[n].fd, &ev, sizeof(ev));
					if (r == sizeof(ev)) {
						/* translate it */
						byte translate_ret =
							LINUXHIDRV_translate(me, &mi->dev[n], dest_ev, &ev);

						/* check */
						if (translate_ret != LIBAROMA_HID_EV_RET_NONE) {
							/* don't process it */
							return translate_ret;
						}
					}
				}
			}
		}
	}
	while (me->internal != NULL);

	/* it was exit message */
	ALOGV("LINUXHIDRV_getinput Input Driver Already Released");
	return LIBAROMA_HID_EV_RET_EXIT;
}

/*
 * function : get input callback - no devices
 */
byte LINUXHIDRV_getinput_nodev(
		LIBAROMA_HIDP me,
		LIBAROMA_HID_EVENTP dest_ev) {
	/* wait for driver release */
	while (me->internal!=NULL){
		libaroma_sleep(100);
	}
	return LIBAROMA_HID_EV_RET_EXIT;
}
/*
 * function : libaroma init hid driver
 */
byte libaroma_hid_driver_init(LIBAROMA_HIDP me) {
	return LINUXHIDRV_init(me);
}

#ifdef __cplusplus
}
#endif
#endif /* __libaroma_linux_hid_driver_c__ */
