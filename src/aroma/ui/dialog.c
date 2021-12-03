/********************************************************************[libaroma]*
 * Copyright (C) 2011-2016 Ahmad Amarullah (http://amarullz.com/)
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
 * Filename		: dialog.c
 * Description : dialog
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 13/01/16 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_dialog_c__
#define __libaroma_dialog_c__
#include <aroma_internal.h>
#include "ui_internal.h"

#ifdef __cplusplus
extern "C" {
#endif
/*
 * Function		: libaroma_dialog_data
 * Return Value: LIBAROMA_DIALOG_DATAP
 * Descriptions: Get dialog data
 */
LIBAROMA_DIALOG_DATAP libaroma_dialog_data(LIBAROMA_WINDOWP win){
	if (!win->client_data){
		return NULL;
	}
	return (LIBAROMA_DIALOG_DATAP) win->client_data;
}

/*
 * Function		: libaroma_dialog_updatebg
 * Return Value: void
 * Descriptions: update window bg
 */
void libaroma_dialog_updatebg(LIBAROMA_WINDOWP win, LIBAROMA_CANVASP c){
	LIBAROMA_DIALOG_DATAP cdata = libaroma_dialog_data(win);
	if (!cdata){
		return;
	}

	/* draw bg */
	libaroma_draw(c,cdata->bg,0,0,0);
	/*
	if (cdata->flags&LIBAROMA_DIALOG_DIM_PARENT){
		libaroma_draw_rect(c,0,0,c->w,c->h,0,0x7c);
	}
	*/
	if (cdata->flags&LIBAROMA_DIALOG_WITH_SHADOW){
		libaroma_draw_shadow(
			c,
			cdata->x+libaroma_dp(2), cdata->y+libaroma_dp(24),
			cdata->w-libaroma_dp(4), cdata->h-libaroma_dp(26),
			libaroma_dp(24),libaroma_dp(32),
			0x40,
			0
		);
	}

	word bgcolor = libaroma_colorget(NULL, win)->dialog_bg;

	/* draw dialog */
	libaroma_gradient(c,
		cdata->x, cdata->y,
		cdata->w, cdata->h,
		bgcolor,bgcolor,
		libaroma_dp(2),
		0x1111
	);
}

/*
 * Function		: libaroma_dialog_window
 * Return Value: LIBAROMA_WINDOWP
 * Descriptions: Create dialog window
 */
LIBAROMA_WINDOWP libaroma_dialog_window(
	int w, int h, byte flags, LIBAROMA_COLORSETP colorset
){
	int min_sz = libaroma_dp(72);
	int iw=w+libaroma_dp(16);
	int ih=h+libaroma_dp(16);

	if ((iw<min_sz)||(ih<min_sz)){
		ALOGW("libaroma_dialog_window: Invalid dialog size");
		return NULL;
	}
	if (iw>libaroma_wm()->w){
		iw=libaroma_wm()->w;
	}
	if (ih>libaroma_wm()->h){
		ih=libaroma_wm()->h;
	}

	LIBAROMA_DIALOG_DATAP cdata = (LIBAROMA_DIALOG_DATAP) calloc(sizeof(LIBAROMA_DIALOG_DATA),1);
	if (!cdata){
		ALOGW("libaroma_dialog_window: Cannot init client data");
		return NULL;
	}

	/* init values */
	cdata->w = iw-libaroma_dp(16);
	cdata->h = ih-libaroma_dp(16);
	cdata->x = (libaroma_wm()->w>>1) - (cdata->w>>1);
	cdata->y = (libaroma_fb()->h>>1) - (cdata->h>>1) - libaroma_wm()->y;
	cdata->flags = flags;

	/* init bg */
	cdata->bg=libaroma_canvas(libaroma_wm()->w,libaroma_wm()->h);
	LIBAROMA_CANVASP wmcv=libaroma_wm_canvas(0,0,libaroma_wm()->w,libaroma_wm()->h);
	// libaroma_draw(cdata->bg,wmcv,0,0,0);
	if (flags&LIBAROMA_DIALOG_DIM_PARENT){
		libaroma_draw_opacity(cdata->bg,wmcv,0,0,0,0x8F);
	}
	else{
		libaroma_draw(cdata->bg,wmcv,0,0,0);
		// libaroma_draw_rect(cdata->bg,0,0,libaroma_wm()->w,libaroma_wm()->h,0,0x70);
	}

	/* create window */
	LIBAROMA_WINDOWP win = libaroma_window(NULL,0,0,libaroma_wm()->w,libaroma_wm()->h);
	if (!win){
		ALOGW("libaroma_dialog_window: Cannot allocate window");
		return NULL;
	}
	if (!win->prev_screen){
		win->prev_screen=libaroma_canvas(libaroma_fb()->w, libaroma_fb()->h);
		libaroma_draw(win->prev_screen, wmcv, libaroma_wm()->x+win->x, libaroma_wm()->y+win->y, 0);
	}
	libaroma_canvas_free(wmcv);
	win->colorset = colorset;
	win->client_data = (voidp) cdata;
	win->onupdatebg	= libaroma_dialog_updatebg;
	return win;
}

/*
 * Function		: libaroma_dialog_free
 * Return Value: byte
 * Descriptions: Free dialog window
 */
byte libaroma_dialog_free(
	LIBAROMA_WINDOWP win
){
	if (win){
		LIBAROMA_WINDOWP parent=NULL;
		if (win->client_data){
			LIBAROMA_DIALOG_DATAP cdata = (LIBAROMA_DIALOG_DATAP) win->client_data;
			parent=cdata->parent;
			libaroma_canvas_free(cdata->bg);
			free(win->client_data);
			win->client_data=NULL;
		}
		byte ret=libaroma_window_aniclose(win, LIBAROMA_WINDOW_SHOW_ANIMATION_FADE, 250);
		if (parent){
			ret=libaroma_window_show(parent);
		}
		return ret;
	}
	return 0;
}

int libaroma_dialog_confirm(
	const char * title,
	const char * message,
	const char * button1,
	const char * button2,
	LIBAROMA_COLORSETP colorset,
	byte flags){

	int dialog_w = libaroma_wm()->w-libaroma_dp(58);
	int text_w = dialog_w-libaroma_dp(48);
	int title_h = 0;
	if (title){
		title_h=libaroma_ctl_label_height(
			title, text_w,
			0, 6,
			LIBAROMA_TEXT_LEFT|
			LIBAROMA_TEXT_FIXED_INDENT|
			LIBAROMA_TEXT_NOHR,
			100
		);
	}
	int msg_h = 0;
	if (message){
		msg_h=libaroma_ctl_label_height(
			message, text_w,
			0, 4,
			LIBAROMA_TEXT_LEFT|
			LIBAROMA_TEXT_FIXED_INDENT|
			LIBAROMA_TEXT_NOHR,
			120
		);
	}
	int dialog_h = libaroma_dp(120)+title_h+msg_h;

	LIBAROMA_WINDOWP win=libaroma_dialog_window(
		dialog_w, dialog_h,
		flags,
		colorset
	);

	LIBAROMA_DIALOG_DATAP cdata = libaroma_dialog_data(win);

	word title_color = libaroma_colorget(NULL, win)->dialog_primary_text;
	word msg_color = libaroma_colorget(NULL, win)->dialog_secondary_text;
	word btncolor = libaroma_colorget(NULL, win)->dialog_primary_text;
	byte button_style = 0;
	if (flags&LIBAROMA_DIALOG_ACCENT_BUTTON){
		btncolor = libaroma_colorget(NULL, win)->accent;
		button_style=LIBAROMA_CTL_BUTTON_COLORED;
	}

	if (title){
		libaroma_ctl_label_valign(win,10,title,
			libaroma_px(cdata->x)+24,
			libaroma_px(cdata->y)+24,
			libaroma_px(text_w),libaroma_px(title_h+10),
			title_color,0,6,
			LIBAROMA_TEXT_LEFT|
			LIBAROMA_TEXT_FIXED_INDENT|
			LIBAROMA_TEXT_NOHR,
			100,10
		);
	}
	if (message){
		libaroma_ctl_label_valign(win,11,message,
			libaroma_px(cdata->x)+24,
			libaroma_px(cdata->y)+44+libaroma_px(title_h),
			libaroma_px(text_w),libaroma_px(msg_h+10),
			msg_color,0,4,
			LIBAROMA_TEXT_LEFT|
			LIBAROMA_TEXT_FIXED_INDENT|
			LIBAROMA_TEXT_NOHR,
			120,10
		);
	}

	int button_y = libaroma_px(cdata->y+cdata->h)-52;

	if (button1){
		int button1_w = libaroma_ctl_button_width(button1);
		int button1_x = libaroma_px(cdata->x+cdata->w-button1_w)-16;
		libaroma_ctl_button(
			win,
			1,
			button1_x,
			button_y,
			libaroma_px(button1_w), 36,
			button1,
			button_style,
			btncolor
		);

		if (button2){
			int button2_w = libaroma_ctl_button_width(button2);
			int button2_x = button1_x-libaroma_px(button2_w);

			libaroma_ctl_button(
				win,
				2,
				button2_x,
				button_y,
				libaroma_px(button2_w), 36,
				button2,
				button_style,
				btncolor
			);
		}
	}
	LIBAROMA_WINDOWP parent=libaroma_wm_get_active_window();
	if (parent)
		cdata->parent=parent;
	
	libaroma_window_anishow(win,LIBAROMA_WINDOW_SHOW_ANIMATION_FADE,250);

	win->onpool=1;
	LIBAROMA_MSG msg;
	dword command;
	byte cmd;
	word id;
	int retval=-1;

	do{
		command=libaroma_window_pool(win,&msg);
		cmd = LIBAROMA_CMD(command);
		id = LIBAROMA_CMD_ID(command);
		if (msg.msg==LIBAROMA_MSG_KEY_BACK){
			if (flags&LIBAROMA_DIALOG_CANCELABLE){
				ALOGV("libaroma_dialog_confirm: Back Button - Cancel");
				win->onpool=0;
			}
		}
		else if (cmd==LIBAROMA_CMD_CLICK){
			if (id==1){
				ALOGV("libaroma_dialog_confirm: Button 1 Selected");
				retval=1;
				win->onpool=0;
				libaroma_sleep(300);
			}
			else if (id==2){
				ALOGV("libaroma_dialog_confirm: Button 2 Selected");
				retval=2;
				win->onpool=0;
				libaroma_sleep(300);
			}
		}
		else if (msg.msg==LIBAROMA_MSG_TOUCH){
			if (flags&LIBAROMA_DIALOG_CANCELABLE){
				if (msg.state==1){
					if (libaroma_window_inside(win, 0, msg.x, msg.y) && ((msg.x<cdata->x)||(msg.y<cdata->y)||(msg.y>cdata->y+cdata->h)||
						(msg.x>cdata->x+cdata->w))){
						ALOGV("libaroma_dialog_confirm: Touch Outside Dialog - Cancel");
						win->onpool=0;
					}
				}
			}
		}
	}
	while(win->onpool);

	libaroma_dialog_free(win);
	return retval;
}

LIBAROMA_CONTROLP libaroma_dialog_progress(
	const char * title,
	const char * message,
	const char * cancelbtn,
	LIBAROMA_COLORSETP colorset,
	byte flags){

	int dialog_w = libaroma_wm()->w-libaroma_dp(58);
	int text_w = dialog_w-libaroma_dp(100);
	int title_h = 0;
	if (title){
		title_h=libaroma_ctl_label_height(
			title, text_w,
			0, 6,
			LIBAROMA_TEXT_LEFT|
			LIBAROMA_TEXT_FIXED_INDENT|
			LIBAROMA_TEXT_NOHR,
			100
		);
	}
	int msg_h = 0;
	if (message){
		msg_h=libaroma_ctl_label_height(
			message, text_w,
			0, 4,
			LIBAROMA_TEXT_LEFT|
			LIBAROMA_TEXT_FIXED_INDENT|
			LIBAROMA_TEXT_NOHR,
			120
		);
	}
	else msg_h=libaroma_dp(48);
	int dialog_h = libaroma_dp(cancelbtn==NULL?84:120)+title_h+msg_h;

	LIBAROMA_WINDOWP win=libaroma_dialog_window(
		dialog_w, dialog_h,
		flags,
		colorset
	);

	LIBAROMA_DIALOG_DATAP cdata = libaroma_dialog_data(win);

	word title_color = libaroma_colorget(NULL, win)->dialog_primary_text;
	word msg_color = libaroma_colorget(NULL, win)->dialog_secondary_text;
	word btncolor = libaroma_colorget(NULL, win)->dialog_primary_text;
	byte button_style = 0;
	if (flags&LIBAROMA_DIALOG_ACCENT_BUTTON){
		btncolor = libaroma_colorget(NULL, win)->accent;
		button_style=LIBAROMA_CTL_BUTTON_COLORED;
	}

	if (title){
		libaroma_ctl_label_valign(win,10,title,
			libaroma_px(cdata->x)+24,
			libaroma_px(cdata->y)+24,
			libaroma_px(text_w),libaroma_px(title_h+10),
			title_color,0,6,
			LIBAROMA_TEXT_LEFT|
			LIBAROMA_TEXT_FIXED_INDENT|
			LIBAROMA_TEXT_NOHR,
			100,10
		);
	}
	if (message){
		libaroma_ctl_label_valign(win,11,message,
			libaroma_px(cdata->x)+84,
			libaroma_px(cdata->y)+44+libaroma_px(title_h),
			libaroma_px(text_w),libaroma_px(msg_h+10),
			msg_color,0,4,
			LIBAROMA_TEXT_LEFT|
			LIBAROMA_TEXT_FIXED_INDENT|
			LIBAROMA_TEXT_NOHR,
			120,10
		);
	}

	int prog_x;
	if (message==NULL)
		prog_x=(libaroma_px(cdata->x)+(libaroma_px(cdata->w/2)-24));
	else prog_x=libaroma_px(cdata->x)+20;
	int prog_y=(libaroma_px(cdata->y)+(libaroma_px(cdata->h/2)-24));
	if (title!=NULL) prog_y+=(44+libaroma_px(title_h))/2;

	LIBAROMA_CONTROLP progress=libaroma_ctl_progress(win, 0, prog_x, prog_y, 48, 48,
								LIBAROMA_CTL_PROGRESS_CIRCULAR|LIBAROMA_CTL_PROGRESS_INDETERMINATE, 1, 1);

	int button_y = libaroma_px(cdata->y+cdata->h)-52;

	if (cancelbtn){
		int button1_w = libaroma_ctl_button_width(cancelbtn);
		int button1_x = libaroma_px(cdata->x+cdata->w-button1_w)-16;
		libaroma_ctl_button(
			win,
			1,
			button1_x,
			button_y,
			libaroma_px(button1_w), 36,
			cancelbtn,
			button_style,
			btncolor
		);
	}

	libaroma_window_anishow(win,LIBAROMA_WINDOW_SHOW_ANIMATION_FADE,250);
/*
	byte onpool=1;
	LIBAROMA_MSG msg;
	dword command;
	byte cmd;
	word id;
	int retval=-1;

	do{
		command=libaroma_window_pool(win,&msg);
		cmd = LIBAROMA_CMD(command);
		id = LIBAROMA_CMD_ID(command);
		if (msg.msg==LIBAROMA_MSG_KEY_BACK){
			if (flags&LIBAROMA_DIALOG_CANCELABLE){
				ALOGV("libaroma_dialog_progress: Back Button - Cancel");
				onpool=0;
			}
		}
		else if (cmd==LIBAROMA_CMD_CLICK){
			if (id==1){
				ALOGV("libaroma_dialog_progress: Cancel Button Selected");
				retval=1;
				onpool=0;
				//libaroma_sleep(300);
			}
		}
		else if (msg.msg==LIBAROMA_MSG_TOUCH){
			if (flags&LIBAROMA_DIALOG_CANCELABLE){
				if (msg.state==1){
					if ((msg.x<cdata->x)||(msg.y<cdata->y)||(msg.y>cdata->y+cdata->h)||
						(msg.x>cdata->x+cdata->w)){
						ALOGV("libaroma_dialog_progress: Touch Outside Dialog - Cancel");
						onpool=0;
					}
				}
			}
		}
	}
	while(onpool);

	libaroma_dialog_free(win);
	return retval;*/
	return progress;
}

byte _libaroma_dialog_list_simple_option_cb(
	LIBAROMA_CONTROLP ctl,
	LIBAROMA_CTL_LIST_ITEMP item,
	int id,
	byte checked,
	voidp data,
	byte state
){
	LIBAROMA_CTL_LIST_ITEMP * currsel = (LIBAROMA_CTL_LIST_ITEMP*) data;
	if (state==LIBAROMA_LISTITEM_CB_SELECTED){
		if (*currsel==NULL){
			*currsel=item;
			return 1;
		}
		else if (*currsel!=item){
			LIBAROMA_CTL_LIST_ITEMP oldsel=*currsel;
			*currsel=NULL;
			libaroma_listitem_set_selected(ctl,oldsel,0);
			*currsel=item;
			return 1;
		}
	}
	else if (state==LIBAROMA_LISTITEM_CB_UNSELECTED){
		if (*currsel!=item){
			return 1;
		}
	}
	return 0;
}

int libaroma_dialog_list(
	const char * title,
	const char * button1,
	const char * button2,
	LIBAROMA_LISTITEM_TEMPLATE templates[],
	size_t items_num,
	byte returnOnSelect,
	LIBAROMA_COLORSETP colorset,
	byte flags){

	int dialog_w = libaroma_wm()->w-libaroma_dp(58);
	int text_w = dialog_w-libaroma_dp(48);
	int title_h = 0;
	if (title){
		title_h=libaroma_ctl_label_height(
			title, text_w,
			0, 6,
			LIBAROMA_TEXT_LEFT|
			LIBAROMA_TEXT_FIXED_INDENT|
			LIBAROMA_TEXT_NOHR,
			100
		);
	}
	int msg_h = title==NULL?0:10;
	int dialog_h = libaroma_dp(100)+title_h+msg_h;

	LIBAROMA_WINDOWP win=libaroma_dialog_window(
		dialog_w, dialog_h,
		flags,
		colorset
	);

	LIBAROMA_DIALOG_DATAP cdata = libaroma_dialog_data(win);

	word title_color = libaroma_colorget(NULL, win)->dialog_primary_text;
	// word msg_color = libaroma_colorget(NULL, win)->dialog_secondary_text;
	word btncolor = libaroma_colorget(NULL, win)->dialog_primary_text;
	byte button_style = 0;
	if (flags&LIBAROMA_DIALOG_ACCENT_BUTTON){
		btncolor = libaroma_colorget(NULL, win)->accent;
		button_style=LIBAROMA_CTL_BUTTON_COLORED;
	}

	LIBAROMA_CONTROLP listc=libaroma_ctl_list(
		win, 1,
		libaroma_px(cdata->x),
		libaroma_px(cdata->y)+44+libaroma_px(title_h),
		libaroma_px(cdata->w),
		10,
		0, 0,
		libaroma_colorget(NULL,win)->control_bg,
		LIBAROMA_CTL_SCROLL_WITH_VBORDER
	);

	LIBAROMA_CTL_LIST_ITEMP selitem=NULL;

	size_t i;
	int last_id = 1;
	for (i=0; i < items_num; i++){
		printf("ITEM N° %d, ID %d, TYPE %i, LAST_ID %d\n", i, templates[i].id, templates[i].type, last_id);
		LIBAROMA_CTL_LIST_ITEMP itm = malloc(sizeof(LIBAROMA_CTL_LIST_ITEM));
		switch (templates[i].type)
		{
			case LIBAROMA_LIST_ITEM_KIND_CHECK:
				 itm=libaroma_listitem_check(
						listc, templates[i].id, templates[i].selected,
						templates[i].title,
						templates[i].message,
						templates[i].image,
						templates[i].flags,	-1);
				break;
			case LIBAROMA_LIST_ITEM_KIND_OPTION:
				 itm=libaroma_listitem_option(
						listc, templates[i].id, templates[i].selected,
						templates[i].title,
						templates[i].message,
						templates[i].image,
						templates[i].flags,	-1);
				break;
			case LIBAROMA_LIST_ITEM_KIND_MENU:
				itm=libaroma_listitem_menu(
						listc, templates[i].id,
						templates[i].title,
						templates[i].message,
						templates[i].image,
						templates[i].flags,	-1);
				break;
			case LIBAROMA_LIST_ITEM_KIND_CAPTION:
				itm=libaroma_listitem_caption(
						listc, templates[i].id,
						templates[i].title, -1);
				break;
			case LIBAROMA_LIST_ITEM_KIND_DIVIDER:
				itm=libaroma_listitem_divider(
						listc, templates[i].id,
						templates[i].flags, -1);
				break;
			case LIBAROMA_LIST_ITEM_KIND_IMAGE:
				itm=libaroma_listitem_image(
						listc, templates[i].id,
						templates[i].image,
						templates[i].imageheight,
						templates[i].flags, -1);
				break;
			case LIBAROMA_LIST_ITEM_KIND_TEXT:
				itm=libaroma_listitem_text(
						listc, templates[i].id,
						templates[i].message, 8, 8,
						LIBAROMA_FONT(templates[i].text_font, templates[i].text_size),
						templates[i].flags, -1);
				break;
		}
		if (!itm) {
			ALOGW("libaroma_dialog_list: invalid item kind");
			free(itm);
			return 0;
		}
		if (templates[i].id > last_id){
			last_id=templates[i].id;
		}
		if (templates[i].selected){
			libaroma_listitem_check_set_cb(
				listc,
				itm,
				_libaroma_dialog_list_simple_option_cb,
				(voidp) &selitem
			);
			libaroma_listitem_set_selected(
				listc,itm,1
			);
		}
	}
	msg_h = libaroma_ctl_scroll_get_height(listc);
	if (msg_h>libaroma_wm()->h-(libaroma_dp(160)+title_h)){
		msg_h=libaroma_wm()->h-(libaroma_dp(160)+title_h);
	}

	dialog_h = libaroma_dp(100)+title_h+msg_h;
	cdata->h = dialog_h;
	cdata->y = (libaroma_wm()->h>>1) - (cdata->h>>1);
	libaroma_control_resize(
		listc,
		libaroma_px(cdata->x),
		libaroma_px(cdata->y)+44+libaroma_px(title_h),
		libaroma_px(cdata->w),
		libaroma_px(msg_h+5)
	);

	if (title){
		libaroma_ctl_label_valign(win,10,title,
			libaroma_px(cdata->x)+24,
			libaroma_px(cdata->y)+24,
			libaroma_px(text_w),libaroma_px(title_h+10),
			title_color,0,6,
			LIBAROMA_TEXT_LEFT|
			LIBAROMA_TEXT_FIXED_INDENT|
			LIBAROMA_TEXT_NOHR,
			100,10
		);
	}
	int button_y = libaroma_px(cdata->y+cdata->h)-52;

	if (button1){
		int button1_w = libaroma_ctl_button_width(button1);
		int button1_x = libaroma_px(cdata->x+cdata->w-button1_w)-16;
		last_id++;
		/*LIBAROMA_CONTROLP button1_ctl = */libaroma_ctl_button(
			win,
			last_id, //last used ID + 1
			button1_x,
			button_y,
			libaroma_px(button1_w), 36,
			button1,
			button_style,
			btncolor
		);
		//printf("Created button1 with theoretical ID %d and real ID %d (both should be the same)\n", last_id, button1_ctl->id);

		if (button2){
			int button2_w = libaroma_ctl_button_width(button2);
			int button2_x = button1_x-libaroma_px(button2_w);
			last_id++;
			/*LIBAROMA_CONTROLP button2_ctl = */libaroma_ctl_button(
				win,
				last_id,
				button2_x,
				button_y,
				libaroma_px(button2_w), 36,
				button2,
				button_style,
				btncolor
			);
		//printf("Created button2 with theoretical ID %d and real ID %d (both should be the same)\n", last_id, button2_ctl->id);
		}
	}


	libaroma_window_anishow(win,LIBAROMA_WINDOW_SHOW_ANIMATION_FADE, 250);
	//libaroma_window_anishow(win,0,0);

	if (selitem){
		libaroma_ctl_list_scroll_to_item(
			listc,
			selitem,
			0
		);
	}

	win->onpool=1;
	LIBAROMA_MSG msg;
	dword command;
	byte cmd;
	word id;
	int retval=-1;

	do{
		command=libaroma_window_pool(win,&msg);
		cmd = LIBAROMA_CMD(command);
		id = LIBAROMA_CMD_ID(command);
		if (msg.msg==LIBAROMA_MSG_KEY_BACK){
			if (flags&LIBAROMA_DIALOG_CANCELABLE){
				ALOGV("libaroma_dialog_confirm: Back Button - Cancel");
				win->onpool=0;
			}
		}
		else if (cmd==LIBAROMA_CMD_CLICK){
			if (button2 && id==last_id) {
				ALOGV("libaroma_dialog_confirm: Button 2 Selected");
				retval=-1;
				win->onpool=0;
			}
			else if (button2 && id==(last_id-1)){
				ALOGV("libaroma_dialog_confirm: Button 1 Selected");
				if (msg.key!=0){
					retval = msg.key;
				}
				else {
					retval=id;
				}
				win->onpool=0;
				//libaroma_sleep(300);
			}
			else if (id==last_id) {
				ALOGV("libaroma_dialog_confirm: Button 1 Selected");
				if (msg.key!=0){
					retval = msg.key;
				}
				else {
					retval=id;
				}
				win->onpool=0;
			}
			else {
				retval = msg.key;
				printf("Dialog returning value: %d\n", retval);
				if (returnOnSelect) { win->onpool = 0; };
			}
		}
		else if (msg.msg==LIBAROMA_MSG_TOUCH){
			if (flags&LIBAROMA_DIALOG_CANCELABLE){
				if (msg.state==1){
					if (libaroma_window_inside(win, 0, msg.x, msg.y) && ((msg.x<cdata->x)||(msg.y<cdata->y)||(msg.y>cdata->y+cdata->h)||
						(msg.x>cdata->x+cdata->w))){
						ALOGV("libaroma_dialog_confirm: Touch Outside Dialog - Cancel");
						win->onpool=0;
					}
				}
			}
		}
		/*printf("Dialog Command = (CMD: %i, ID: %i, Msg.Key: %i), i: %d\n",
			LIBAROMA_CMD(command),
			LIBAROMA_CMD_ID(command),
			msg.key,
			i);*/
	}
	while(win->onpool);

	libaroma_dialog_free(win);
	return retval;
}

#ifdef __cplusplus
}
#endif
#endif /* __libaroma_dialog_c__ */


