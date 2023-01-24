/********************************************************************[libaroma]*
 * Copyright (C) 2011-2015 Ahmad Amarullah (http://amarullz.com/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *______________________________________________________________________________
 *
 * Filename    : hid.h
 * Description : hid handler
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 21/01/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_aroma_h__
  #error "Include <aroma.h> instead."
#endif
#ifndef __libaroma_hid_h__
#define __libaroma_hid_h__

/* hid macros */
#define LIBAROMA_HID_KEYCODE_MAX      0x200
#define LIBAROMA_HID_TOUCH_KEYCODE    (LIBAROMA_HID_KEYCODE_MAX-1)
#define LIBAROMA_HID_WHEEL_KEYCODE    (LIBAROMA_HID_KEYCODE_MAX-2)
#define LIBAROMA_HID_LMOUSE_KEYCODE   (LIBAROMA_HID_KEYCODE_MAX-3)
#define LIBAROMA_HID_RMOUSE_KEYCODE   (LIBAROMA_HID_KEYCODE_MAX-4)

/* Input Event Type */
#define LIBAROMA_HID_EV_TYPE_NONE     0x00 /* Won't processed */
#define LIBAROMA_HID_EV_TYPE_KEY      0x01 /* Keypad/Keyboards */
#define LIBAROMA_HID_EV_TYPE_TOUCH    0x02 /* Touch Screen */
#define LIBAROMA_HID_EV_TYPE_MOUSE    0x03 /* Simulated Touch Screen */

/* Get Input Return Values */
#define LIBAROMA_HID_EV_RET_NONE      0x00 /* ignore  */
#define LIBAROMA_HID_EV_RET_SELECT    0x01 /* enter, select.. */
#define LIBAROMA_HID_EV_RET_MENU      0x02 /* menu / search button, .. */
#define LIBAROMA_HID_EV_RET_BACK      0x03 /* back, esc, .. */
#define LIBAROMA_HID_EV_RET_UP        0x04 /* up */
#define LIBAROMA_HID_EV_RET_DOWN      0x05 /* down */
#define LIBAROMA_HID_EV_RET_TOUCH     0x06 /* touch */
#define LIBAROMA_HID_EV_RET_RAWKEY    0x07 /* raw keycode */
#define LIBAROMA_HID_EV_RET_POWER     0x08 /* power */
#define LIBAROMA_HID_EV_RET_VOLDOWN   0x09 /* vol down */
#define LIBAROMA_HID_EV_RET_VOLUP     0x0a /* vol up */
#define LIBAROMA_HID_EV_RET_MOUSE     0x0b /* mouse */
#define LIBAROMA_HID_EV_RET_EXIT      0xcc /* halt */
#define LIBAROMA_HID_EV_RET_ERROR     0xdd /* error */

/* Input Event State */
#define LIBAROMA_HID_EV_STATE_UP      0x00 /* key/touch up */
#define LIBAROMA_HID_EV_STATE_DOWN    0x01 /* key/touch down */
#define LIBAROMA_HID_EV_STATE_MOVE    0x02 /* move */
#define LIBAROMA_HID_EV_STATE_CANCEL  0x03 /* canceled */

/*
 * Typedef     : LIBAROMA_HID
 * Descriptions: hid type structure
 */
typedef struct _LIBAROMA_HID LIBAROMA_HID;
typedef struct _LIBAROMA_HID * LIBAROMA_HIDP;

/*
 * Typedef     : LIBAROMA_HID_INITIALIZER
 * Descriptions: Runtime HID Initializer Callback
 */
typedef byte (*LIBAROMA_HID_INITIALIZER)(LIBAROMA_HIDP);

/*
 * Function    : libaroma_hid_set_initializer
 * Return Value: byte
 * Descriptions: Set hid initializer callback
 */
byte libaroma_hid_set_initializer(LIBAROMA_HID_INITIALIZER cb);

/*
 * Typedef     : LIBAROMA_HID_EVENT
 * Descriptions: hid event type structure
 */
typedef struct _LIBAROMA_HID_EVENT LIBAROMA_HID_EVENT;
typedef struct _LIBAROMA_HID_EVENT * LIBAROMA_HID_EVENTP;

/*
 * Structure   : _LIBAROMA_HID
 * Typedef     : LIBAROMA_HID, * LIBAROMA_HIDP
 * Descriptions: hid structure
 */
struct _LIBAROMA_HID {
  voidp internal;
  
  /* driver callbacks */
  void (*release)(LIBAROMA_HIDP);
  byte (*getinput)(LIBAROMA_HIDP, LIBAROMA_HID_EVENTP);
  byte (*config)(LIBAROMA_HIDP, const char *, const char *, dword);
  
  /* libaroma usable data */
  int   screen_width;
  int   screen_height;
  byte  key_pressed[LIBAROMA_HID_KEYCODE_MAX / 8];
  byte  has_mice;
  
  /* touch move informations */
  int   touch_last_x;
  int   touch_last_y;
};

/*
 * Structure   : _LIBAROMA_HID_EVENT
 * Typedef     : LIBAROMA_HID_EVENT, * LIBAROMA_HID_EVENTP
 * Descriptions: hid structure
 */
struct _LIBAROMA_HID_EVENT {
  byte  type;
  int   key;
  byte  state;
  int   x;
  int   y;
};

/*
 * Function    : libaroma_hid_set_keypress
 * Return Value: byte
 * Descriptions: set key press status
 */
byte libaroma_hid_set_keypress(int code, byte state);

/*
 * Function    : libaroma_hid_get_keypress
 * Return Value: byte
 * Descriptions: get key press status
 */
byte libaroma_hid_get_keypress(int code);

/*
 * Function    : libaroma_hid_touch_pressed
 * Return Value: byte
 * Descriptions: is touchscreen pressed
 */
byte libaroma_hid_touch_pressed();

/*
 * Function    : libaroma_hid_get
 * Return Value: byte
 * Descriptions: get hid event
 */
byte libaroma_hid_get(LIBAROMA_HID_EVENTP e);

/*
 * Function    : libaroma_hid_config
 * Return Value: byte
 * Descriptions: runtime config
 */
byte libaroma_hid_config(char * name,char * svalue,dword dvalue);

/*
 * Function		: libaroma_hid_has_mice
 * Return Value: byte
 * Descriptions: is mice available?
 */
byte libaroma_hid_has_mice();

/*
 * Function		: libaroma_hid_last_input_x
 * Return Value: int
 * Descriptions: get last touch/mice x coord
 */
int libaroma_hid_last_input_x();

/*
 * Function		: libaroma_hid_last_input_y
 * Return Value: int
 * Descriptions: get last touch/mice y coord
 */
int libaroma_hid_last_input_y();


#endif /* __libaroma_hid_h__ */
