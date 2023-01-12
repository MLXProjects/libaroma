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
 * Filename		: ctl_list.h
 * Description : list control
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 04/03/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_aroma_h__
	#error "Include <aroma.h> instead."
#endif
#ifndef __libaroma_ctl_list_h__
#define __libaroma_ctl_list_h__

/* flag aliases */
#define LIBAROMA_CTL_LIST_NO_INDICATOR			LIBAROMA_CTL_SCROLL_NO_INDICATOR
#define LIBAROMA_CTL_LIST_WITH_SHADOW			LIBAROMA_CTL_SCROLL_WITH_SHADOW
#define LIBAROMA_CTL_LIST_WITH_HANDLE			LIBAROMA_CTL_SCROLL_WITH_HANDLE

/* item flags */
#define LIBAROMA_CTL_LIST_ITEM_RECEIVE_TOUCH				0x0001
#define LIBAROMA_CTL_LIST_ITEM_REGISTER_THREAD				0x0002
#define LIBAROMA_LISTITEM_WITH_SEPARATOR					0x0004
#define LIBAROMA_LISTITEM_SEPARATOR_TEXTALIGN				0x0008


#define LIBAROMA_LIST_ITEM_KIND_CAPTION						0x0001
#define LIBAROMA_LIST_ITEM_KIND_CHECK						0x0002
#define LIBAROMA_LIST_ITEM_KIND_DIVIDER						0x0003
#define LIBAROMA_LIST_ITEM_KIND_IMAGE						0x0004
#define LIBAROMA_LIST_ITEM_KIND_MENU						0x0005
#define LIBAROMA_LIST_ITEM_KIND_OPTION						0x0006
#define LIBAROMA_LIST_ITEM_KIND_TEXT						0x0007

/* item message */
#define LIBAROMA_CTL_LIST_ITEM_MSG_TOUCH_DOWN				0x1
#define LIBAROMA_CTL_LIST_ITEM_MSG_TOUCH_UP					0x2
#define LIBAROMA_CTL_LIST_ITEM_MSG_TOUCH_MOVE				0x3
#define LIBAROMA_CTL_LIST_ITEM_MSG_TOUCH_CANCEL				0x4
#define LIBAROMA_CTL_LIST_ITEM_MSG_TOUCH_HOLDED				0x5
#define LIBAROMA_CTL_LIST_ITEM_MSG_THREAD					0x10
#define LIBAROMA_CTL_LIST_ITEM_MSG_ACTIVATED				0x11
#define LIBAROMA_CTL_LIST_ITEM_MSG_INACTIVATED				0x12

/* message param */
#define LIBAROMA_CTL_LIST_ITEM_MSGPARAM_HOLDED				0x10

/* draw state */
#define LIBAROMA_CTL_LIST_ITEM_DRAW_NORMAL					0x0
#define LIBAROMA_CTL_LIST_ITEM_DRAW_PUSHED					0x1
#define LIBAROMA_CTL_LIST_ITEM_DRAW_FOCUSED					0x2
#define LIBAROMA_CTL_LIST_ITEM_DRAW_CACHE					0x4
#define LIBAROMA_CTL_LIST_ITEM_DRAW_ADDONS					0x10

/* message status */
#define LIBAROMA_CTL_LIST_ITEM_MSGRET_NEED_DRAW				0x1
#define LIBAROMA_CTL_LIST_ITEM_MSGRET_HANDLED				0x2
#define LIBAROMA_CTL_LIST_ITEM_MSGRET_UNREG_THREAD			0x4
#define LIBAROMA_CTL_LIST_ITEM_MSGRET_HAVE_ADDONS_DRAW		0x8

/* check if items are the same one (internal is unique) */
#define libaroma_ctl_list_items_equal(item1, item2) \
		(item1->internal==item2->internal)

/* list item */
typedef struct _LIBAROMA_CTL_LIST_ITEM LIBAROMA_CTL_LIST_ITEM;
typedef struct _LIBAROMA_CTL_LIST_ITEM * LIBAROMA_CTL_LIST_ITEMP;

/* item handler */
typedef struct{
	byte (*message)
		(LIBAROMA_CONTROLP,LIBAROMA_CTL_LIST_ITEMP, byte, dword, int, int);
	void (*draw)
		(LIBAROMA_CONTROLP,LIBAROMA_CTL_LIST_ITEMP,LIBAROMA_CANVASP,word,byte);
	void (*destroy)
		(LIBAROMA_CONTROLP,LIBAROMA_CTL_LIST_ITEMP);
} LIBAROMA_CTL_LIST_ITEM_HANDLER,
* LIBAROMA_CTL_LIST_ITEM_HANDLERP;

/* list touch event */
typedef struct{
	int start_x;
	int start_y;
	int last_x;
	int last_y;
} LIBAROMA_CTL_LIST_TOUCHPOS, * LIBAROMA_CTL_LIST_TOUCHPOSP;

/* item state */
typedef struct{
	byte normal_handler;
	LIBAROMA_RIPPLE ripple;
	LIBAROMA_CANVASP cache_rest;
	LIBAROMA_CANVASP cache_push;
	LIBAROMA_CANVASP cache_client;
} LIBAROMA_CTL_LIST_ITEM_STATE,
	* LIBAROMA_CTL_LIST_ITEM_STATEP;

/* item structure */
struct _LIBAROMA_CTL_LIST_ITEM{
	int y;
	int h;
	int id;
	voidp internal;
	word flags;
	LIBAROMA_CTL_LIST_ITEM_HANDLERP handler;
	LIBAROMA_CTL_LIST_ITEM_STATEP state;
	LIBAROMA_CTL_LIST_ITEMP next;
	LIBAROMA_CTL_LIST_ITEMP prev;
	LIBAROMA_CONTROLP list;
	byte is_leftcontrol;
	int checkwidth;
	int checkx;
	/* if you want to hold any data within a list item, use this */
	void *tag;
};

/*
 * Function		: libaroma_ctl_list
 * Return Value: LIBAROMA_CONTROLP
 * Descriptions: create list scroll control
 */
LIBAROMA_CONTROLP libaroma_ctl_list(
		LIBAROMA_WINDOWP win, word id,
		int x, int y, int w, int h,
		int horizontal_padding,
		int vertical_padding,
		word bg_color, byte flags
);

byte libaroma_ctl_list_init_opt_groups(LIBAROMA_CONTROLP list);
LIBAROMA_STACKP libaroma_ctl_list_get_groups(LIBAROMA_CONTROLP ctl);
#define libaroma_ctl_list_get_group_count(list) \
		libaroma_ctl_list_get_groups(list)->n

/*
 * Function		: libaroma_ctl_list_get_item_internal
 * Return Value: LIBAROMA_CTL_LIST_ITEMP
 * Descriptions: get item at index or id
 */
LIBAROMA_CTL_LIST_ITEMP libaroma_ctl_list_get_item_internal(
	LIBAROMA_CONTROLP ctl, int index, byte find_id
);

/*
 * Function		: libaroma_ctl_list_get_item_count
 * Return Value: int
 * Descriptions: get listitem's item count
 */
int libaroma_ctl_list_get_item_count(
	LIBAROMA_CONTROLP ctl
);

/*
 * Function		: libaroma_ctl_list_invalidate_item
 * Return Value: void
 * Descriptions: do draw item directly - public
 */
void libaroma_ctl_list_invalidate_item(
		LIBAROMA_CONTROLP ctl,
		LIBAROMA_CTL_LIST_ITEMP item
);

/*
 * Function		: libaroma_ctl_list_get_touched_item
 * Return Value: LIBAROMA_CTL_LIST_ITEMP
 * Descriptions: get touched item
 */
LIBAROMA_CTL_LIST_ITEMP libaroma_ctl_list_get_touched_item(
	LIBAROMA_CONTROLP ctl
);

/*
 * Function		: libaroma_ctl_list_get_focused_item
 * Return Value: LIBAROMA_CTL_LIST_ITEMP
 * Descriptions: get focused item
 */
LIBAROMA_CTL_LIST_ITEMP libaroma_ctl_list_get_focused_item(
	LIBAROMA_CONTROLP ctl
);

/*
 * Function		: libaroma_ctl_list_get_first_item
 * Return Value: LIBAROMA_CTL_LIST_ITEMP
 * Descriptions: get first item
 */
LIBAROMA_CTL_LIST_ITEMP libaroma_ctl_list_get_first_item(
	LIBAROMA_CONTROLP ctl
);

/*
 * Function		: libaroma_ctl_list_get_last_item
 * Return Value: LIBAROMA_CTL_LIST_ITEMP
 * Descriptions: get last item
 */
LIBAROMA_CTL_LIST_ITEMP libaroma_ctl_list_get_last_item(
	LIBAROMA_CONTROLP ctl
);

/*
 * Function		: libaroma_ctl_list_getpos
 * Return Value: LIBAROMA_CTL_LIST_TOUCHPOSP
 * Descriptions: get touch positions
 */
LIBAROMA_CTL_LIST_TOUCHPOSP libaroma_ctl_list_getpos(LIBAROMA_CONTROLP ctl);

/*
 * Function		: libaroma_ctl_list_del_itemp_internal
 * Return Value: byte
 * Descriptions: del item from list
 */
byte libaroma_ctl_list_del_itemp_internal(
	LIBAROMA_CONTROLP ctl, LIBAROMA_CTL_LIST_ITEMP f
);

/*
 * Function		: libaroma_ctl_list_del_item_internal
 * Return Value: byte
 * Descriptions: del list item by id/index
 */
byte libaroma_ctl_list_del_item_internal(
	LIBAROMA_CONTROLP ctl, int index, byte find_id
);

/*
 * Function		: libaroma_ctl_list_is_valid
 * Return Value: byte
 * Descriptions: check control, if it was valid list control
 */
byte libaroma_ctl_list_is_valid(LIBAROMA_CONTROLP ctl);

/*
 * Function		: libaroma_ctl_list_item_setheight
 * Return Value: byte
 * Descriptions: update item height
 */
byte libaroma_ctl_list_item_setheight(
		LIBAROMA_CONTROLP ctl, LIBAROMA_CTL_LIST_ITEMP item, int h);

/*
 * Function		: libaroma_ctl_list_add_item_internal
 * Return Value: LIBAROMA_CTL_LIST_ITEMP
 * Descriptions: add item internally
 */
LIBAROMA_CTL_LIST_ITEMP libaroma_ctl_list_add_item_internal(
		LIBAROMA_CONTROLP ctl,
		int id,
		int height,
		word flags,
		voidp internal,
		LIBAROMA_CTL_LIST_ITEM_HANDLERP handler,
		int at_index);

#define libaroma_ctl_list_isactive(ctl) libaroma_ctl_scroll_isactive(ctl)

/*
 * Function		: libaroma_listitem_nonitem
 * Return Value: byte
 * Descriptions: is non item
 */
byte libaroma_listitem_nonitem(LIBAROMA_CTL_LIST_ITEMP item);

/*
 * Function		: libaroma_ctl_list_item_position
 * Return Value: byte
 * Descriptions: get item position
 */
byte libaroma_ctl_list_item_position(
		LIBAROMA_CONTROLP ctl,LIBAROMA_CTL_LIST_ITEMP item,
		LIBAROMA_RECTP rect, byte absolute);

/*
 * Function		: libaroma_ctl_list_scroll_to_item
 * Return Value: byte
 * Descriptions: focus item to scroll
 */
byte libaroma_ctl_list_scroll_to_item(
		LIBAROMA_CONTROLP ctl,
		LIBAROMA_CTL_LIST_ITEMP item,
		byte smooth
	);

#endif /* __libaroma_ctl_list_h__ */
