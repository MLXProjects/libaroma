#ifndef __libaroma_ctl_clock_c__
#define __libaroma_ctl_clock_c__
#include <aroma_internal.h>
#include "../ui/ui_internal.h"
#include <stdio.h>
#include <math.h>
#include <time.h>
#define MYPI 3.14159265358979323846

dword _libaroma_ctl_clock_msg(LIBAROMA_CONTROLP, LIBAROMA_MSGP);
void _libaroma_ctl_clock_draw(LIBAROMA_CONTROLP, LIBAROMA_CANVASP);
byte _libaroma_ctl_clock_destroy(LIBAROMA_CONTROLP);
byte _libaroma_ctl_clock_thread(LIBAROMA_CONTROLP);

void gettime(int *hour, int *min, int *sec)
{
	time_t now = time(NULL);
	struct tm *tm_struct = localtime(&now);
	*hour = tm_struct->tm_hour;
	*min = tm_struct->tm_min;
	*sec = tm_struct->tm_sec;
}

static LIBAROMA_CONTROL_HANDLER _libaroma_ctl_clock_handler = {
	message : _libaroma_ctl_clock_msg,
	draw : _libaroma_ctl_clock_draw,
	focus : NULL,
	destroy : _libaroma_ctl_clock_destroy,
	thread : _libaroma_ctl_clock_thread
};

typedef struct{
	/* colors */
	word bgcolor;
	word hcolor;
	word mcolor;
	word scolor;
	word bcolor;
	word ccolor;

	/* time */
	int hour;
	int min;
	int sec;

	/* ui */
	byte flags;
	byte alpha;
	byte redraw;
	LIBAROMA_CANVASP bg;
	LIBAROMA_RIPPLEP ripple;

	LIBAROMA_MUTEX mutex;
} _LIBAROMA_CTL_CLOCK, * _LIBAROMA_CTL_CLOCKP;

dword _libaroma_ctl_clock_msg(LIBAROMA_CONTROLP ctl, LIBAROMA_MSGP msg){
	/* internal check */
	_LIBAROMA_CTL_CHECK(
		_libaroma_ctl_clock_handler, _LIBAROMA_CTL_CLOCKP, 0);

	switch (msg->msg){
		case LIBAROMA_MSG_WIN_ACTIVE:
		case LIBAROMA_MSG_WIN_INACTIVE:
		case LIBAROMA_MSG_WIN_RESIZE:{
			libaroma_mutex_lock(me->mutex);
			me->redraw = 1;
			libaroma_mutex_unlock(me->mutex);
		} break;
		case LIBAROMA_MSG_TOUCH:{
			/* touch handler */
			if (msg->state == LIBAROMA_HID_EV_STATE_DOWN){
				if (me->flags&LIBAROMA_CTL_CLOCK_RIPPLE){
					libaroma_ripple_down(me->ripple, msg->x, msg->y);
				}
			}
			else if (msg->state == LIBAROMA_HID_EV_STATE_MOVE){
				if (me->flags&LIBAROMA_CTL_CLOCK_RIPPLE){
					libaroma_ripple_move(me->ripple, msg->x, msg->y);
				}
			}
			else if (msg->state == LIBAROMA_HID_EV_STATE_UP){
				if (me->flags&LIBAROMA_CTL_CLOCK_RIPPLE){
					byte ret = libaroma_ripple_up(me->ripple, 0);
					if ((ret&LIBAROMA_RIPPLE_TOUCHED) && !(ret&LIBAROMA_RIPPLE_HOLDED)){
						libaroma_window_post_command_ex(
							LIBAROMA_CMD_SET(LIBAROMA_CMD_CLICK, 0, ctl->id),
															0, 0, 0, ctl
						);
					}
				}
				else {
					libaroma_window_post_command_ex(
						LIBAROMA_CMD_SET(LIBAROMA_CMD_CLICK, 0, ctl->id),
													0, 0, 0, ctl
					);
				}
			}
		} break;
	}
	return 0;
}

LIBAROMA_CONTROLP libaroma_ctl_clock(
	LIBAROMA_WINDOWP win,
	word id, int x,	int y,
	int w, int h,
	LIBAROMA_CANVASP bg,
	byte flags
){
	/* init internal data */
	_LIBAROMA_CTL_CLOCKP me = calloc(sizeof(_LIBAROMA_CTL_CLOCK), 1);
	if (!me){
		ALOGW("libaroma_ctl_clock alloc clock memory failed");
		return NULL;
	}

	/* set internal data */
	libaroma_mutex_init(me->mutex);
	gettime(&me->hour, &me->min, &me->sec);
	me->hcolor=RGB(342756);
	me->mcolor=RGB(348856);
	me->scolor=RGB(349999);
	me->bcolor=RGB(10DCA0);
	me->bgcolor=RGB(0);
	me->ccolor=RGB(FF00FF);
	me->bg=(bg==NULL)?NULL:libaroma_canvas_dup(bg); /* duplicate bg canvas */
	me->alpha=(flags&LIBAROMA_CTL_CLOCK_ALPHA)?0x7F:0xFF;
	me->flags=flags;
	
	if (flags&LIBAROMA_CTL_CLOCK_RIPPLE){
		LIBAROMA_RIPPLEP ripple = calloc(sizeof(LIBAROMA_RIPPLE), 1);
		if (!ripple){
			ALOGW("libaroma_ctl_clock alloc ripple failed");
			free(me);
			return 0;
		}
		me->ripple=ripple;
	}

	/* init control */
	LIBAROMA_CONTROLP ctl = libaroma_control_new(id, x, y, w, h,
												 libaroma_dp(48), libaroma_dp(48), /* min size */
												 (voidp)me, &_libaroma_ctl_clock_handler, win);
	if (!ctl)	{
		libaroma_mutex_free(me->mutex);
		free(me);
		return NULL;
	}
	me->redraw = 1;
	return ctl;
} /* End of libaroma_ctl_clock */

byte libaroma_ctl_clock_setalpha(LIBAROMA_CONTROLP ctl, byte alpha){
	_LIBAROMA_CTL_CHECK(_libaroma_ctl_clock_handler, _LIBAROMA_CTL_CLOCKP, 0);
	libaroma_mutex_lock(me->mutex);
	if (alpha!=me->alpha) me->redraw=1; /* redraw on alpha update */
	if (!alpha) me->alpha=0xFF;
	else me->alpha=alpha;
	libaroma_mutex_unlock(me->mutex);
	return 1;
}

byte libaroma_ctl_clock_setcolor(LIBAROMA_CONTROLP ctl, byte type, word color){
	_LIBAROMA_CTL_CHECK(_libaroma_ctl_clock_handler, _LIBAROMA_CTL_CLOCKP, 0);
	libaroma_mutex_lock(me->mutex);
	switch (type){
		case LIBAROMA_CTL_CLOCK_COLOR_HOUR:
			me->hcolor=color;
			break;
		case LIBAROMA_CTL_CLOCK_COLOR_MINUTE:
			me->mcolor=color;
			break;
		case LIBAROMA_CTL_CLOCK_COLOR_SECOND:
			me->scolor=color;
			break;
		case LIBAROMA_CTL_CLOCK_COLOR_BORDER:
			me->bcolor=color;
			break;
		case LIBAROMA_CTL_CLOCK_COLOR_CENTER:
			me->ccolor=color;
			break;
	}
	libaroma_mutex_unlock(me->mutex);
	return 1;
}

byte libaroma_ctl_clock_setborder(LIBAROMA_CONTROLP ctl, byte enable){
	_LIBAROMA_CTL_CHECK(_libaroma_ctl_clock_handler, _LIBAROMA_CTL_CLOCKP, 0);
	libaroma_mutex_lock(me->mutex);
	if (enable && me->flags&LIBAROMA_CTL_CLOCK_NOBORDER)
		me->flags &= ~LIBAROMA_CTL_CLOCK_NOBORDER;
	else if (!enable&&!(me->flags&LIBAROMA_CTL_CLOCK_NOBORDER))
		me->flags |= LIBAROMA_CTL_CLOCK_NOBORDER;
	libaroma_mutex_unlock(me->mutex);
	return 1;
}

byte libaroma_ctl_clock_setbg_ex(
	LIBAROMA_CONTROLP ctl,
	LIBAROMA_CANVASP bg, byte use_orig
){
	_LIBAROMA_CTL_CHECK(_libaroma_ctl_clock_handler, _LIBAROMA_CTL_CLOCKP, 0);
	libaroma_mutex_lock(me->mutex);
	if (me->bg!=NULL && (me->flags&LIBAROMA_CTL_CLOCK_FREEBG))
		libaroma_canvas_free(me->bg);
	if (bg->w <= ctl->w && bg->h <= ctl->h)
		me->bg=use_orig?bg:libaroma_canvas_dup(bg);
	else {
		me->bg=libaroma_canvas(ctl->w, ctl->h);
		libaroma_draw_scale_smooth(me->bg, bg, 0, 0, ctl->w, ctl->h, 0, 0, bg->w, bg->h);
	}
	if (!use_orig && (me->flags&LIBAROMA_CTL_CLOCK_FREEBG))
		me->flags &= ~LIBAROMA_CTL_CLOCK_FREEBG;
	me->redraw=1;
	libaroma_mutex_unlock(me->mutex);
	return 1;
}

void getLocationOnCircles(
	int cx, int cy,
	int radius, int angle,
	int *outx, int *outy
){
	float tmp = angle * (2 * MYPI / 360);
	if (angle >= 0 && angle <= 90){
		*outx = cx + radius * sin(tmp);
		*outy = cy - radius * cos(tmp);
	}
	else if (angle > 90 && angle <= 180){
		tmp = tmp - MYPI / 2;
		*outx = cx + radius * cos(tmp);
		*outy = cy + radius * sin(tmp);
	}
	else if (angle > 180 && angle <= 270){
		tmp = tmp - MYPI;
		*outx = cx - radius * sin(tmp);
		*outy = cy + radius * cos(tmp);
	}
	else if (angle > 270 && angle <= 360){
		tmp = tmp - (MYPI + MYPI / 2);
		*outx = cx - radius * cos(tmp);
		*outy = cy - radius * sin(tmp);
	}
	//ALOGD("location on x:%d  y:%d", *outx, *outy);
}

void _libaroma_ctl_clock_draw(LIBAROMA_CONTROLP ctl, LIBAROMA_CANVASP c){
	_LIBAROMA_CTL_CHECK(_libaroma_ctl_clock_handler, _LIBAROMA_CTL_CLOCKP, 0);
	libaroma_control_erasebg(ctl, c);
	
	libaroma_draw(c, me->bg, (c->w-me->bg->w)/2, (c->h-me->bg->h)/2, 1);
		
	int sz = MIN(c->w >> 1, c->h >> 1);
	int outx, outy;
	if (!(me->flags&LIBAROMA_CTL_CLOCK_NOBORDER))
		libaroma_draw_arc(c, sz, sz, sz, sz, 15, 0, 360, me->bcolor,
					me->alpha, 0, 0.8);
	int tmphour = me->hour > 12 ? me->hour - 12 : me->hour;
	/* draw hour line */
	getLocationOnCircles(sz, sz, (sz/3)*2, tmphour * 1.0 / 12 * 360, &outx, &outy);
	libaroma_draw_line_width(c, sz, sz, outx, outy, 13, me->hcolor,
					me->alpha, 0, 0.5);
	/* draw min line */
	getLocationOnCircles(sz, sz, sz, me->min * 1.0 / 60 * 360, &outx, &outy);
	libaroma_draw_line_width(c, sz, sz, outx, outy, 8, me->mcolor,
						me->alpha, 0, 0.5);
	/* draw sec line */
	getLocationOnCircles(sz, sz, (sz/8)*7, me->sec * 1.0 / 60 * 360, &outx, &outy);
	libaroma_draw_line_width(c, sz, sz, outx, outy, 5, me->scolor,
						me->alpha, 0, 0.5);

	libaroma_draw_circle(c, me->ccolor, sz, sz, 20, 0xff);
	if (me->flags&LIBAROMA_CTL_CLOCK_RIPPLE){
		int ripple_i = 0;
		int ripple_p = 0;
		while(libaroma_ripple_loop(me->ripple,&ripple_i,&ripple_p)){
			int x=0;
			int y=0;
			int size=0;
			byte push_opacity=0;
			byte ripple_opacity=0;
			if (libaroma_ripple_calculation(
				me->ripple, c->w, c->h, &push_opacity, &ripple_opacity,
				&x, &y, &size,ripple_p
			)){
				libaroma_draw_circle(
					c, libaroma_color_isdark(me->bgcolor)?RGB(FFFFFF):0, x, y, size, push_opacity
				);
			}
		}
	}
}

byte _libaroma_ctl_clock_destroy(LIBAROMA_CONTROLP ctl){
	/* internal check */
	_LIBAROMA_CTL_CHECK(_libaroma_ctl_clock_handler, _LIBAROMA_CTL_CLOCKP, 0);
	if (me->bg!=NULL && (me->flags&LIBAROMA_CTL_CLOCK_FREEBG))
		libaroma_canvas_free(me->bg);
	libaroma_mutex_unlock(me->mutex);
	libaroma_mutex_free(me->mutex);
	free(me);
	return 1;
}

byte _libaroma_ctl_clock_thread(LIBAROMA_CONTROLP ctl){
	_LIBAROMA_CTL_CHECK(_libaroma_ctl_clock_handler, _LIBAROMA_CTL_CLOCKP, 0);
	byte ret=0;
	if (me->flags&LIBAROMA_CTL_CLOCK_RIPPLE){
		byte res = libaroma_ripple_thread(me->ripple, 0);
		if (res&LIBAROMA_RIPPLE_REDRAW){
			me->redraw=1;
		}
		if (res&LIBAROMA_RIPPLE_HOLDED){
			libaroma_window_post_command_ex(
				LIBAROMA_CMD_SET(LIBAROMA_CMD_HOLD, 0, ctl->id),
											0, 0, 0, ctl
			);
		}
	}
	int hour = 0, min = 0, sec = 0;
	gettime(&hour, &min, &sec);
	libaroma_mutex_lock(me->mutex);
	if (me->sec != sec){ /* hour and time also may change at second change */
		me->hour = hour;
		me->min = min;
		me->sec = sec;
		me->redraw = 1;
	}
	if (me->redraw){
		me->redraw = 0;
		ret=1;
	}
	libaroma_mutex_unlock(me->mutex);
	return ret;
}

#endif /* __libaroma_ctl_clock_c__ */
