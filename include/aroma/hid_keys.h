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
 * Filename    : hid_keys.h
 * Description : hid default keycodes
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 24/01/23 - Author(s): Michael Jauregui
 *
 */
#ifndef __libaroma_aroma_h__
  #error "Include <aroma.h> instead."
#endif
#ifndef __libaroma_hid_keys_h__
#define __libaroma_hid_keys_h__

/* minimal standard keyboard codes
 * based on uapi/linux/input-event-codes.h */
#define LIBAROMA_HID_KEY_UNKNOWN		0
#define LIBAROMA_HID_KEY_ESC			1 
#define LIBAROMA_HID_KEY_1				2
#define LIBAROMA_HID_KEY_2				3
#define LIBAROMA_HID_KEY_3				4
#define LIBAROMA_HID_KEY_4				5
#define LIBAROMA_HID_KEY_5				6
#define LIBAROMA_HID_KEY_6				7
#define LIBAROMA_HID_KEY_7				8
#define LIBAROMA_HID_KEY_8				9
#define LIBAROMA_HID_KEY_9				10
#define LIBAROMA_HID_KEY_0				11
#define LIBAROMA_HID_KEY_MINUS			12
#define LIBAROMA_HID_KEY_EQUAL			13
#define LIBAROMA_HID_KEY_BACKSPACE		14
#define LIBAROMA_HID_KEY_TAB			15
#define LIBAROMA_HID_KEY_Q				16
#define LIBAROMA_HID_KEY_W				17
#define LIBAROMA_HID_KEY_E				18
#define LIBAROMA_HID_KEY_R				19
#define LIBAROMA_HID_KEY_T				20
#define LIBAROMA_HID_KEY_Y				21
#define LIBAROMA_HID_KEY_U				22
#define LIBAROMA_HID_KEY_I				23
#define LIBAROMA_HID_KEY_O				24
#define LIBAROMA_HID_KEY_P				25
#define LIBAROMA_HID_KEY_LBRACE			26
#define LIBAROMA_HID_KEY_RBRACE			27
#define LIBAROMA_HID_KEY_ENTER			28
#define LIBAROMA_HID_KEY_LCTRL			29
#define LIBAROMA_HID_KEY_A				30
#define LIBAROMA_HID_KEY_S				31
#define LIBAROMA_HID_KEY_D				32
#define LIBAROMA_HID_KEY_F				33
#define LIBAROMA_HID_KEY_G				34
#define LIBAROMA_HID_KEY_H				35
#define LIBAROMA_HID_KEY_J				36
#define LIBAROMA_HID_KEY_K				37
#define LIBAROMA_HID_KEY_L				38
#define LIBAROMA_HID_KEY_SEMICOLON		39
#define LIBAROMA_HID_KEY_APOSTROPHE		40
#define LIBAROMA_HID_KEY_GRAVE			41
#define LIBAROMA_HID_KEY_LSHIFT			42
#define LIBAROMA_HID_KEY_BACKSLASH		43
#define LIBAROMA_HID_KEY_Z				44
#define LIBAROMA_HID_KEY_X				45
#define LIBAROMA_HID_KEY_C				46
#define LIBAROMA_HID_KEY_V				47
#define LIBAROMA_HID_KEY_B				48
#define LIBAROMA_HID_KEY_N				49
#define LIBAROMA_HID_KEY_M				50
#define LIBAROMA_HID_KEY_COMMA			51
#define LIBAROMA_HID_KEY_DOT			52
#define LIBAROMA_HID_KEY_SLASH			53
#define LIBAROMA_HID_KEY_RSHIFT			54
#define LIBAROMA_HID_KEY_KPASTERISK		55
#define LIBAROMA_HID_KEY_LALT			56
#define LIBAROMA_HID_KEY_SPACE			57
#define LIBAROMA_HID_KEY_CAPSLOCK		58
#define LIBAROMA_HID_KEY_F1				59
#define LIBAROMA_HID_KEY_F2				60
#define LIBAROMA_HID_KEY_F3				61
#define LIBAROMA_HID_KEY_F4				62
#define LIBAROMA_HID_KEY_F5				63
#define LIBAROMA_HID_KEY_F6				64
#define LIBAROMA_HID_KEY_F7				65
#define LIBAROMA_HID_KEY_F8				66
#define LIBAROMA_HID_KEY_F9				67
#define LIBAROMA_HID_KEY_F10			68
#define LIBAROMA_HID_KEY_NUMLOCK		69
#define LIBAROMA_HID_KEY_SCROLLLOCK		70
#define LIBAROMA_HID_KEY_KP7			71
#define LIBAROMA_HID_KEY_KP8			72
#define LIBAROMA_HID_KEY_KP9			73
#define LIBAROMA_HID_KEY_KPMINUS		74
#define LIBAROMA_HID_KEY_KP4			75
#define LIBAROMA_HID_KEY_KP5			76
#define LIBAROMA_HID_KEY_KP6			77
#define LIBAROMA_HID_KEY_KPPLUS			78
#define LIBAROMA_HID_KEY_KP1			79
#define LIBAROMA_HID_KEY_KP2			80
#define LIBAROMA_HID_KEY_KP3			81
#define LIBAROMA_HID_KEY_KP0			82
#define LIBAROMA_HID_KEY_KPDOT			83
#define LIBAROMA_HID_KEY_KPENTER		96
#define LIBAROMA_HID_KEY_RCTRL			97
#define LIBAROMA_HID_KEY_KPSLASH		98
#define LIBAROMA_HID_KEY_SYSRQ			99
#define LIBAROMA_HID_KEY_RALT			100
#define LIBAROMA_HID_KEY_LINEFEED		101
#define LIBAROMA_HID_KEY_HOME			102
#define LIBAROMA_HID_KEY_UP				103
#define LIBAROMA_HID_KEY_PAGEUP			104
#define LIBAROMA_HID_KEY_LEFT			105
#define LIBAROMA_HID_KEY_RIGHT			106
#define LIBAROMA_HID_KEY_END			107
#define LIBAROMA_HID_KEY_DOWN			108
#define LIBAROMA_HID_KEY_PAGEDOWN		109
#define LIBAROMA_HID_KEY_INSERT			110
#define LIBAROMA_HID_KEY_DELETE			111
#define LIBAROMA_HID_KEY_VOLUMEDOWN		114
#define LIBAROMA_HID_KEY_VOLUMEUP		115
#define LIBAROMA_HID_KEY_POWER			116
#define LIBAROMA_HID_KEY_PAUSE			119
#define LIBAROMA_HID_KEY_KPCOMMA		121
#define LIBAROMA_HID_KEY_LMETA			125
#define LIBAROMA_HID_KEY_RMETA			126

/* check alias keycodes */
#define LIBAROMA_HID_KEY_ISCTRL(key)		(key==LIBAROMA_HID_KEY_LCTRL	|| key==LIBAROMA_HID_KEY_RCTRL)
#define LIBAROMA_HID_KEY_ISALT(key)			(key==LIBAROMA_HID_KEY_LALT	|| key==LIBAROMA_HID_KEY_RALT)
#define LIBAROMA_HID_KEY_ISSHIFT(key)		(key==LIBAROMA_HID_KEY_LSHIFT|| key==LIBAROMA_HID_KEY_RSHIFT)
#define LIBAROMA_HID_KEY_ISBACK(key)		(key==LIBAROMA_HID_KEY_ESC)
#define LIBAROMA_HID_KEY_ISSELECT(key)		(key==LIBAROMA_HID_KEY_ENTER	|| key==LIBAROMA_HID_KEY_KPENTER)
#define LIBAROMA_HID_KEY_ISMETA(key)		(key==LIBAROMA_HID_KEY_LMETA || key==LIBAROMA_HID_KEY_RMETA)

#endif /* __libaroma_hid_keys_h__ */
