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
 * Filename		: ctl_slider.c
 * Description : slider control
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 24/11/21 - Author(s): Michael Jauregui
 *
 */
#ifndef __libaroma_ctl_slider_c__
#define __libaroma_ctl_slider_c__
#include <aroma_internal.h>

#ifdef __cplusplus
extern "C" {
#endif
/* HANDLER */
dword _libaroma_ctl_slider_msg(LIBAROMA_CONTROLP, LIBAROMA_MSGP);
void _libaroma_ctl_slider_draw (LIBAROMA_CONTROLP, LIBAROMA_CANVASP);
void _libaroma_ctl_slider_destroy(LIBAROMA_CONTROLP);
byte _libaroma_ctl_slider_thread(LIBAROMA_CONTROLP);
static LIBAROMA_CONTROL_HANDLER _libaroma_ctl_slider_handler={
	.message = _libaroma_ctl_slider_msg,
	.draw = _libaroma_ctl_slider_draw,
	.focus = NULL,
	.destroy = _libaroma_ctl_slider_destroy,
	.thread = _libaroma_ctl_slider_thread
};

/*
 * Structure	 : __LIBAROMA_CTL_SLIDER
 * Typedef		 : _LIBAROMA_CTL_SLIDER, * _LIBAROMA_CTL_SLIDERP
 * Descriptions: slider structure
 */
typedef struct __LIBAROMA_CTL_SLIDER _LIBAROMA_CTL_SLIDER;
typedef struct __LIBAROMA_CTL_SLIDER * _LIBAROMA_CTL_SLIDERP;
struct __LIBAROMA_CTL_SLIDER{
	LIBAROMA_MUTEX mutex;
	
	byte type;
	float max;
	int value;
	float preval;
	float curval;
	long tick;
	long rtick;
	float state;
	float currstate;
	float rstate;
	byte rshow;
	int timing;
	
	byte update;
	
	void (*onchange)(LIBAROMA_CONTROLP, int value);
};

/*
 * Function		: _libaroma_ctl_slider_thread
 * Return Value: byte
 * Descriptions: control thread callback
 */
byte _libaroma_ctl_slider_thread(LIBAROMA_CONTROLP ctl) {
	_LIBAROMA_CTL_SLIDERP me = (_LIBAROMA_CTL_SLIDERP) ctl->internal;
	if (me->state<1.0){
		me->state = libaroma_control_state(
			me->tick, 200
		);
		float newstate = libaroma_cubic_bezier_swiftout(me->state);
		me->curval = me->preval+((me->value - me->preval) * newstate);
		if (me->currstate!=me->state){
			me->currstate =me->state;
			return 1;
		}
	}
	if (me->rshow || (me->rstate>0.0)){
		me->rstate = libaroma_motion_accelerate(
			libaroma_control_state(
				me->rtick, 200
			)	
		);
		if (!me->rshow){
			me->rstate = 1.0 - me->rstate;
		}
		return 1;
	}
	if (me->update){
		me->update=0;
		return 1;
	}
	return 0;
} /* End of _libaroma_ctl_slider_thread */

/*
 * Function		: _libaroma_ctl_slider_update
 * Return Value: void
 * Descriptions: animated slider update
 */
void _libaroma_ctl_slider_update(
		LIBAROMA_CONTROLP ctl,
		_LIBAROMA_CTL_SLIDERP me){
	me->preval = me->curval;
	me->tick=libaroma_tick();
	me->state=0.0;
} /* End of _libaroma_ctl_slider_update */

/*
 * Function		: _libaroma_ctl_slider_destroy
 * Return Value: void
 * Descriptions: control destroy callback
 */
void _libaroma_ctl_slider_destroy(
		LIBAROMA_CONTROLP ctl){
	/* internal check */
	_LIBAROMA_CTL_CHECK(
		_libaroma_ctl_slider_handler, _LIBAROMA_CTL_SLIDERP,
	);
	libaroma_mutex_free(me->mutex);
	free(me);
} /* End of _libaroma_ctl_slider_destroy */

/*
 * Function		: _libaroma_ctl_slider_draw
 * Return Value: void
 * Descriptions: control draw callback
 */
void _libaroma_ctl_slider_draw(
		LIBAROMA_CONTROLP ctl,
		LIBAROMA_CANVASP c){
	/* internal check */
	_LIBAROMA_CTL_CHECK(
		_libaroma_ctl_slider_handler, _LIBAROMA_CTL_SLIDERP,
	);

	libaroma_control_erasebg(ctl,c);
	
	int ix = libaroma_dp(20);
	int iw = ctl->w-(ix*2);
	int ih = libaroma_dp(4);
	int iy = ctl->h>>1;
	float val_w = ((float)iw * me->curval) / me->max;
	int h_sz = libaroma_dp(20);
	int h_x = ix+val_w;
	int sh_sz= libaroma_dp(22);
	/* draw full track */
	libaroma_draw_line_width(c,
		ix, iy, ix+iw, iy, ih, 
		libaroma_colorget(ctl,NULL)->control_bg, 0xFF, 0, 0.5
	);
	if (val_w>0){
		int val_h = libaroma_dp(6);
		/* draw value track */
		libaroma_draw_line_width(c,
			ix, iy, h_x, iy, val_h,
			libaroma_colorget(ctl,NULL)->primary, 0xFF, 0, 0.5
		);
	}
	/* handle ripple */
	libaroma_draw_circle(c, libaroma_colorget(ctl,NULL)->primary, h_x, iy, h_sz+(h_sz*me->rstate), 0x70);
	/* shadow */
	LIBAROMA_CANVASP sh_mask = libaroma_canvas_ex(sh_sz, sh_sz,1);
	libaroma_canvas_setcolor(sh_mask,0,0);
	libaroma_gradient(sh_mask,0,0,sh_sz,sh_sz,0,0,sh_sz>>1,0x1111);
	LIBAROMA_CANVASP sh_cv = libaroma_blur_ex(sh_mask,libaroma_dp(1),1,0);
	libaroma_canvas_free(sh_mask);
	libaroma_draw_opacity(c, sh_cv, h_x-(sh_sz>>1)-libaroma_dp(1), iy-(sh_sz>>1), 3, 0x60);
	libaroma_canvas_free(sh_cv);
	/* handle */
	libaroma_draw_circle(c, libaroma_colorget(ctl,NULL)->primary, h_x, iy, h_sz, 0xFF);
	
} /* End of _libaroma_ctl_slider_draw */

/*
 * Function		: _libaroma_ctl_slider_msg
 * Return Value: dword
 * Descriptions: control message callback
 */
dword _libaroma_ctl_slider_msg(
		LIBAROMA_CONTROLP ctl,
		LIBAROMA_MSGP msg){
	/* internal check */
	_LIBAROMA_CTL_CHECK(
		_libaroma_ctl_slider_handler, _LIBAROMA_CTL_SLIDERP, 0
	);

	switch(msg->msg){
		case LIBAROMA_MSG_WIN_ACTIVE:
		case LIBAROMA_MSG_WIN_INACTIVE:
		case LIBAROMA_MSG_WIN_RESIZE:{
			me->currstate = 0.0;
		} break;
		case LIBAROMA_MSG_TOUCH:{
			float new_value=-1.0;
			int x = msg->x;
			int y = msg->y;
			libaroma_window_calculate_pos(NULL,ctl,&x,&y);
			int ix = libaroma_dp(20);
			int iw = ctl->w-(ix*2);
			int h_sz= ix;
			x-=ix;
			if (x>ix+iw) x-=ix;
			switch (msg->state){
				case LIBAROMA_HID_EV_STATE_DOWN:{
					/* enable ripple circle */
					me->rshow=1;
					me->rtick = libaroma_tick();
					/* if touched outside of handle bounds, update directly */
					if (x < (me->curval-(h_sz>>1)) || x > (me->curval+(h_sz>>1))){
						new_value=(x * me->max) / iw;
					}
				} break;
				case LIBAROMA_HID_EV_STATE_MOVE:{
					new_value=(x * me->max) / iw;
					
				} break;
				case LIBAROMA_HID_EV_STATE_UP:{
					me->rshow=0;
					me->rtick = libaroma_tick();
					new_value=(x * me->max) / iw;
					
				} break;
			}
			if (new_value>-1.0){
				if (new_value>me->max) // fix value
					new_value=me->max;
				if (new_value != me->value){
					me->curval=me->value=new_value;
					me->update=1;
					if (me->onchange!=NULL)
						me->onchange(ctl, new_value);
				}
			}
		} break;
	}
	return 1;
} /* End of _libaroma_ctl_slider_msg */

/*
 * Function		: libaroma_ctl_slider
 * Return Value: LIBAROMA_CONTROLP
 * Descriptions: create new slider control
 */
LIBAROMA_CONTROLP libaroma_ctl_slider(
		LIBAROMA_WINDOWP win,
		word id,
		int x, int y, int w, int h,
		int max,
		int value
){
	/* init internal data */
	_LIBAROMA_CTL_SLIDERP me = (_LIBAROMA_CTL_SLIDERP)
			calloc(sizeof(_LIBAROMA_CTL_SLIDER),1);
	if (!me){
		ALOGW("libaroma_ctl_slider alloc slider memory failed");
		return NULL;
	}
	
	libaroma_mutex_init(me->mutex);
	libaroma_mutex_lock(me->mutex);

	/* set internal data */
	me->max	= max;
	me->value =value;
	me->preval=value;
	me->curval=value;
	me->onchange = NULL;
	me->state= 1;
	me->timing=300;
	libaroma_mutex_unlock(me->mutex);

	/* init control */
	LIBAROMA_CONTROLP ctl =
		libaroma_control_new(
			id,
			x, y, w, h,
			libaroma_dp(48),libaroma_dp(40), /* min size */
			me,
			&_libaroma_ctl_slider_handler,
			win
		);

	if (!ctl){
		free(me);
	}
	return ctl;
} /* End of libaroma_ctl_slider */

/*
 * Function		: libaroma_ctl_slider_timing
 * Return Value: byte
 * Descriptions: set slider animation timing
 */
byte libaroma_ctl_slider_timing(
		LIBAROMA_CONTROLP ctl,
		int timing
){
	/* internal check */
	_LIBAROMA_CTL_CHECK(
		_libaroma_ctl_slider_handler, _LIBAROMA_CTL_SLIDERP, 0
	);
	me->timing = timing;
	return 1;
} /* End of libaroma_ctl_slider_timing */

/*
 * Function		: libaroma_ctl_slider_type
 * Return Value: byte
 * Descriptions: set slider type
 */
byte libaroma_ctl_slider_type(
		LIBAROMA_CONTROLP ctl,
		byte type
){
	/* internal check */
	_LIBAROMA_CTL_CHECK(
		_libaroma_ctl_slider_handler, _LIBAROMA_CTL_SLIDERP, 0
	);
	me->type = type;
	_libaroma_ctl_slider_update(ctl,me);
	return 1;
} /* End of libaroma_ctl_slider_type */

/*
 * Function		: libaroma_ctl_slider_value
 * Return Value: byte
 * Descriptions: set slider value
 */
byte libaroma_ctl_slider_value(
		LIBAROMA_CONTROLP ctl,
		int value
){
	/* internal check */
	_LIBAROMA_CTL_CHECK(
		_libaroma_ctl_slider_handler, _LIBAROMA_CTL_SLIDERP, 0
	);
	if (value>me->max){
		value=me->max;
	}
	me->value = value;
	_libaroma_ctl_slider_update(ctl,me);
	if (me->onchange != NULL){
		me->onchange(ctl, me->value);
	}
	return 1;
} /* End of libaroma_ctl_slider_value */

/*
 * Function		: libaroma_ctl_slider_max
 * Return Value: byte
 * Descriptions: set slider max value
 */
byte libaroma_ctl_slider_max(
		LIBAROMA_CONTROLP ctl,
		int max
){
	/* internal check */
	_LIBAROMA_CTL_CHECK(
		_libaroma_ctl_slider_handler, _LIBAROMA_CTL_SLIDERP, 0
	);
	me->max = max;
	if (me->value>me->max){
		me->value=me->max;
	}
	_libaroma_ctl_slider_update(ctl,me);
	return 1;
} /* End of libaroma_ctl_slider_max */

/*
 * Function		: libaroma_ctl_slider_set_update_callback
 * Return Value: byte
 * Descriptions: set slider update callback
 */
byte libaroma_ctl_slider_set_onchange(
	LIBAROMA_CONTROLP ctl, 
	void *onchange
){
	/* internal check */
	_LIBAROMA_CTL_CHECK(
		_libaroma_ctl_slider_handler, _LIBAROMA_CTL_SLIDERP, 0
	);
	libaroma_mutex_lock(me->mutex);
	me->onchange = onchange;
	libaroma_mutex_unlock(me->mutex);
	return 1;
} /* End of libaroma_ctl_slider_set_update_callback */

/*
 * Function		: libaroma_ctl_slider_timing
 * Return Value: int
 * Descriptions: get slider current animation timing
 */
int libaroma_ctl_slider_get_timing(
		LIBAROMA_CONTROLP ctl
){
	/* internal check */
	_LIBAROMA_CTL_CHECK(
		_libaroma_ctl_slider_handler, _LIBAROMA_CTL_SLIDERP, 0
	);
	return me->timing;
} /* End of libaroma_ctl_slider_get_timing */

/*
 * Function		: libaroma_ctl_slider_value
 * Return Value: int
 * Descriptions: get slider current value
 */
int libaroma_ctl_slider_get_value(
		LIBAROMA_CONTROLP ctl
){
	/* internal check */
	_LIBAROMA_CTL_CHECK(
		_libaroma_ctl_slider_handler, _LIBAROMA_CTL_SLIDERP, 0
	);
	return me->value;
} /* End of libaroma_ctl_slider_get_value */

/*
 * Function		: libaroma_ctl_slider_get_max
 * Return Value: int
 * Descriptions: get slider max value
 */
int libaroma_ctl_slider_get_max(
		LIBAROMA_CONTROLP ctl
){
	/* internal check */
	_LIBAROMA_CTL_CHECK(
		_libaroma_ctl_slider_handler, _LIBAROMA_CTL_SLIDERP, 0
	);
	return me->max;
} /* End of libaroma_ctl_slider_get_max */

/*
 * Function		: libaroma_ctl_slider_get_type
 * Return Value: byte
 * Descriptions: get slider type
 */
byte libaroma_ctl_slider_get_type(
		LIBAROMA_CONTROLP ctl
){
	/* internal check */
	_LIBAROMA_CTL_CHECK(
		_libaroma_ctl_slider_handler, _LIBAROMA_CTL_SLIDERP, 0
	);
	return me->type;
} /* End of libaroma_ctl_slider_get_type */

#ifdef __cplusplus
}
#endif
#endif /* __libaroma_ctl_slider_c__ */
