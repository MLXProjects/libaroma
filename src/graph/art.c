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
 * Filename		: artworker.c
 * Description : artworker
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 06/04/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_artworker_c__
#define __libaroma_artworker_c__
#include <aroma_internal.h>
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Variable		: libaroma_art_svg_logo
 * Type			: char[]
 * Descriptions: aroma logo - svg format
 */
char libaroma_art_svg_logo[] =
	"<svg version=\"1.0\" xmlns=\"http://www.w3.org/2000/svg\" width=\"412px\" height=\"372px\" viewBox=\"0 0 412 372\" preserveAspectRatio=\"xMidYMid meet\">"
		"<g fill=\"#ffffff\">"
			"<path d=\"M281.5 340.3 c-1.1 -0.2 -8.2 -1.6 -15.7 -3 -11.5 -2.2 -13.8 -2.9 -13.8 -4.4 0 -0.9 2.1 -5.3 4.6 -9.5 14.8 -25 24.7 -34.5 60.6 -57.6 8.3 -5.4 14.2 -9.8 13.1 -9.8 -1 0 -2.3 0.4 -2.9 0.8 -0.5 0.5 -2.9 1.5 -5.4 2.2 -20.8 6.5 -52.2 28.7 -73.3 51.8 -3.3 3.7 -6.2 6 -6.8 5.6 -1.5 -0.9 -0.5 -14.8 2.1 -29.2 5 -27.8 13.8 -40.4 33.2 -47.7 9.5 -3.5 16.9 -5 44.3 -9 13.2 -1.9 31.3 -4.8 40.3 -6.3 15.2 -2.6 16.4 -2.7 17.2 -1.1 0.7 1.2 -0.7 5.7 -5.4 16.5 -3.5 8.2 -8.9 22.1 -12.1 30.9 -10.9 30 -17.2 43.1 -26.1 53.9 -10.3 12.5 -20.4 16.7 -39.8 16.5 -6.6 -0.1 -13 -0.3 -14.1 -0.6z\"/>"
			"<path d=\"M183 334.3 c-12.7 -1.4 -38.4 -5.5 -48.5 -7.8 -55.6 -12.5 -81.2 -37.2 -92.4 -89 -5.7 -26.2 -7.2 -48.3 -7.6 -111.5 -0.4 -56.3 -1.2 -68.9 -5.5 -80.6 -2.8 -7.4 -3.2 -14 -0.9 -13.8 0.9 0.1 24.6 13.8 52.5 30.6 60.3 36.1 63.2 37.8 92.9 54.3 29 16.2 42 26.3 53.6 41.8 13 17.4 18.3 34.9 18.2 60.2 -0.1 21.3 -5 44.8 -15.3 73.8 -4 11 -4.4 11.8 -6.7 11.5 -2.1 -0.3 -3.6 -3 -12.2 -21.8 -18.7 -41.3 -32.2 -64.1 -55.4 -93.4 -8.6 -10.9 -62.1 -64.6 -64.4 -64.6 -2.9 0 -0.8 3.4 10.4 16.7 37.4 44.3 71.1 97.7 81.7 129.5 5.6 16.9 10.6 45.5 10.6 61 0 3.8 0 3.8 -3.7 3.6 -2.1 0 -5.4 -0.3 -7.3 -0.5z\"/>"
		"</g>"
	"</svg>";

/*
 * Variable		: __libaroma_art_arrowdrawer_curves_*
 * Type			: float *
 * Descriptions: arrow drawer curves
 */
static const float __libaroma_art_arrowdrawer_curves_top[]={
	 18, 70, 54,	4,160,-19,208, 60, /* top */
	208, 60,239, 99,214,211,127,207,
	222, 70,240,126,221,202,147,214,
	147,214, 97,228, 31,193, 30,113
};
static const float __libaroma_art_arrowdrawer_curves_mid[]={
	 18,120, 18, 66, 66, 23,120, 23, /* middle */
	120, 23,168, 23,211, 72,211,120,
	222,120,222,175,175,210,120,210,
	120,210, 76,210, 38,164, 38,120
};
static const float __libaroma_art_arrowdrawer_curves_bot[]={
	 18,170,	7,148,	2, 84, 31, 60, /* bottom */
	 31, 60, 45, 45, 98, 18,127, 32,
	222,170,208,194,163,240, 96,216,
	 96,216, 61,200, 30,162, 30,127
};

/*
 * Function		: _libaroma_art_arrowdrawer_line
 * Return Value: void
 * Descriptions: internal draw arrow drawer data into canvas
 */
void _libaroma_art_arrowdrawer_line(
		LIBAROMA_CANVASP canvas,
		const float * cb,
		float t,
		int dx, int dy,
		int size,
		byte swap,
		word color,
		byte alpha,
		byte is_mask,
		float aliasing){
	if (!canvas){
		return;
	}
	if (swap){
		t=1-t;
	}
	int pos = (t<=0.5)?0:8;
	if (t<=0.5){
		t*=2;
	}
	else{
		t=(t-0.5) * 2;
	}
	const float * a	= cb + pos;
	const float * b	= cb + pos + 16;
	float x, y, x2, y2;
	libaroma_path_curve_calc(
		t, &x, &y, a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]
	);
	libaroma_path_curve_calc(
		t, &x2,&y2,b[0],b[1],b[2],b[3],b[4],b[5],b[6],b[7]
	);
	if (swap){
		y=240-y;
		y2=240-y2;
	}

	/* scaling */
	float scaling = ((float) size) / 240.0;
	x*=scaling;
	y*=scaling;
	x2*=scaling;
	y2*=scaling;

	/* draw polygonal line */
	libaroma_draw_line_width(
		canvas,
		dx+x, dy+y, dx+x2, dy+y2,
		20*scaling,
		color, alpha, is_mask, aliasing
	);
} /* End of _libaroma_art_arrowdrawer_line */

/*
 * Function		: libaroma_art_arrowdrawer
 * Return Value: byte
 * Descriptions: draw arrow drawer
 */
byte libaroma_art_arrowdrawer(
	LIBAROMA_CANVASP dest,
	float t,
	byte from_arrow,
	int dx,
	int dy,
	int size,
	word color,
	byte alpha,
	byte is_mask,
	float aliasing){
	if ((t<0)||(t>1)){
		return 0;
	}
	if (!dest){
		dest=libaroma_fb()->canvas;
	}
	if ((is_mask)&&(dest->alpha==NULL)){
		return 0;
	}
	if ((!is_mask)&&(alpha<1)){
		return 1;
	}

	/* top line */
	_libaroma_art_arrowdrawer_line(
		dest,
		__libaroma_art_arrowdrawer_curves_top,
		t, dx, dy, size,
		from_arrow, color, alpha, is_mask, aliasing
	);

	/* middle line */
	_libaroma_art_arrowdrawer_line(
		dest,
		__libaroma_art_arrowdrawer_curves_mid,
		t, dx, dy, size,
		from_arrow, color, alpha, is_mask, aliasing
	);

	/* bottom line */
	_libaroma_art_arrowdrawer_line(
		dest,
		__libaroma_art_arrowdrawer_curves_bot,
		t, dx, dy, size,
		from_arrow, color, alpha, is_mask, aliasing
	);

	return 1;
} /* End of libaroma_art_arrowdrawer */


/*
 * Function		: libaroma_art_busy_progress
 * Return Value: LIBAROMA_CANVASP
 * Descriptions: create busy progress sprite canvas
 */
LIBAROMA_CANVASP libaroma_art_busy_progress(
		word basecolor) {
	int i, j;

	/* calculate size */
	int dp1	 = libaroma_dp(1);
	int dp36	= libaroma_dp(36);
	int dp72	= libaroma_dp(72);
	int dp144 = dp72 * 2;
	int dp288 = dp144 * 2;
	int dp28	= libaroma_dp(28);
	int dp56	= dp28 * 2;
	int dp116 = dp144 - dp28;

	/* main + temp canvas */
	LIBAROMA_CANVASP load	= libaroma_canvas_ex(dp72 * 13, dp72, 1);
	if (!load){
		return NULL;
	}
	LIBAROMA_CANVASP load1 = libaroma_canvas_ex(dp288, dp288, 1);
	if (!load1){
		libaroma_canvas_free(load);
		return NULL;
	}
	LIBAROMA_CANVASP load2 = libaroma_canvas_ex(dp56, dp56, 1);
	if (!load2){
		libaroma_canvas_free(load1);
		libaroma_canvas_free(load);
		return NULL;
	}

	/* cleanup */
	libaroma_canvas_setcolor(load, 0x0000, 0);

	/* frame loop */
	for (j = 0; j < 13; j++) {
		/* angle per frame */
		double added = ((27.69230769230769 * j) / 360);

		/* cleanup load1 */
		libaroma_canvas_setcolor(load1, 0x0000, 0);

		/* circle draw */
		for (i = 0; i < 12; i++) {
			/* position */
			double angle = 2 * __PI * ((((double) i) / 12.0) + added);
			int xpos		 = round(dp116 * cos(angle));
			int ypos		 = round(dp116 * sin(angle));

			/* cleanup load2 */
			libaroma_canvas_setcolor(load2, 0x0000, 0);
			int b = 2;

			/* draw */
			libaroma_gradient_ex(
				load2,
				b * dp1,
				b * dp1,
				dp56 - (b * dp1 * 2),
				dp56 - (b * dp1 * 2),
				basecolor,
				basecolor,
				dp28,
				0x1111,
				MIN(0xff, ((i + 1) * 18) + 39),
				MIN(0xff, ((i + 1) * 18))
			);

			/* Stretch Copy to load1 */
			libaroma_draw_scale_smooth(
				load1,
				load2,
				dp144 + xpos - dp28,
				dp144 + ypos - dp28,
				dp56,
				dp56,
				0,
				0,
				dp56,
				dp56
			);
		}

		/* Stretch Copy to load canvas */
		libaroma_draw_scale_smooth(
			load,
			load1,
			j * dp72,
			0,
			dp72,
			dp72,
			0,
			0,
			dp288,
			dp288
		);
	}

	/* free temp canvases */
	libaroma_canvas_free(load1);
	libaroma_canvas_free(load2);

	/* return canvas */
	LIBAROMA_CANVASP load_out =
		libaroma_canvas_ex(dp36 * 13, dp36, 1);
	if (!load_out){
		libaroma_canvas_free(load);
		return NULL;
	}
	libaroma_canvas_setcolor(load_out, 0x0000, 0);

	/* draw */
	libaroma_draw_scale_smooth(
		load_out, load,
		0, 0, dp36 * 13, dp36,
		0, 0, dp72 * 13, dp72
	);
	libaroma_canvas_free(load);
	return load_out;
} /* End of libaroma_art_busy_progress */

/*
 * Function		: libaroma_art_draw_switch_animation
 * Return Value: LIBAROMA_CANVASP
 * Descriptions: draw switch animation frame into canvas
 */
LIBAROMA_CANVASP libaroma_art_draw_switch_animation(
	byte animation,
	//byte side,
	LIBAROMA_CANVASP target_canvas,
	LIBAROMA_CANVASP from,
	LIBAROMA_CANVASP to,
	int fromx, int fromy, int fromw, int fromh,
	int tox, int toy, int tow, int toh,
	float state
){
	// if no animation, or switching from nothing to nothing, return draw target
	if (!animation || ((!from) && (!to)))
		return target_canvas;
	if (!from) from=libaroma_canvas(to->w, to->h);
	if (!to) to=libaroma_canvas(from->w, from->h);
	if (!target_canvas){
		libaroma_canvas(MAX(from->w, to->w), MAX(from->h, to->h));
	}
	int scr_lx,	//screen_leftx, x to draw left window on screen
		scr_ly=0,
		scr_lw,	//screen_leftwidth, width for left window on screen
		scr_lh=target_canvas->h,
		scr_rx,	//screen_rightx, x to draw right window on screen
		scr_ry=0,
		scr_rw,	//screen_rightwidth, width for right window on screen
		scr_rh=target_canvas->h;

	scr_lx=0;
	scr_rw=((float)target_canvas->w*state);//c->w-remain;
	scr_lw=target_canvas->w-scr_rw;
	scr_rx=scr_lw;
/*
	ALOGI("ARTWORKER - CANVAS INFORMATIONS\n"
			"target: w=%d, h=%d\n"
			"from: w=%d, h=%d, x=%d, y=%d\n"
			"to: w=%d, h=%d, x=%d, y=%d",
			target_canvas->w, target_canvas->h,
			from->w, from->h, fromx, fromy,
			to->w, to->h, tox, toy);
*/
	switch (animation){
		case LIBAROMA_ART_SWITCH_ANIMATION_SCALE:
		case LIBAROMA_ART_SWITCH_ANIMATION_STACKIN:
		case LIBAROMA_ART_SWITCH_ANIMATION_STRETCH:{
			if (animation!=LIBAROMA_ART_SWITCH_ANIMATION_STRETCH){
				//if animation is scale or stack in, change Y values first
				float scr_ratio;
				scr_ratio=((float)target_canvas->h/(float)target_canvas->w);
				if (animation!=LIBAROMA_ART_SWITCH_ANIMATION_STACKIN){
					//if animation isn't stack in, set height and y for left image
					scr_lh=scr_lw*scr_ratio;
					scr_ly=(target_canvas->h-scr_lh)/2;
				}
				scr_rh=scr_rw*scr_ratio;
				scr_ry=(target_canvas->h-scr_rh)/2;
				//fill bg with black
				libaroma_canvas_setcolor(target_canvas, 0, 0xFF);
			}
			if (animation==LIBAROMA_ART_SWITCH_ANIMATION_STACKIN){
				libaroma_draw_ex(target_canvas, from, scr_lx, scr_ly, fromx+scr_rw, fromy, scr_lw, scr_lh, 0, 0xFF);
				float bstate=(255.0 * state);
				byte bbstate = (byte) round(bstate);
				LIBAROMA_CANVASP temp_cv=libaroma_canvas(scr_rw, scr_rh);
				libaroma_draw_scale(temp_cv, to, 0, 0, scr_rw, scr_rh, tox, toy, tow, toh, 0);
				libaroma_draw_ex(target_canvas, temp_cv, scr_rx, scr_ry, 0, 0, scr_rw, scr_rh, 1, bbstate);
				libaroma_canvas_free(temp_cv);
				return target_canvas;
			}
			libaroma_draw_scale(
				target_canvas,	//dest
				from,			//src
				scr_lx,			//destx
				scr_ly,			//desty
				scr_lw,			//destw
				scr_lh,			//desth
				fromx,			//srcx
				fromy,			//srcy
				fromw,			//srcw
				fromh,			//srch
				0				//is smooth?
			);
			libaroma_draw_scale(
				target_canvas,	//dest
				to,				//src
				scr_rx,			//destx
				scr_ry,			//desty
				scr_rw,			//destw
				scr_rh,			//desth
				tox,			//srcx
				toy,			//srcy
				tow,			//srcw
				toh,			//srch
				0				//is smooth?
			);
		}
		break;
		case LIBAROMA_ART_SWITCH_ANIMATION_STRETCH_IN:{
			libaroma_canvas_setcolor(target_canvas, 0, 0xFF);
			if (state<0.5){
				scr_lw=target_canvas->w-(((float)target_canvas->w)*(state*2.0));
				scr_lx=(from->w-scr_lw)/2;
				//ALOGI("Left page: y=%d, h=%d, state=%1.2f", scr_ly, scr_lh, (state*2.0));
				libaroma_draw_scale(
					target_canvas,	//dest
					from,			//src
					scr_lx,			//destx
					scr_ly-2,		//desty
					scr_lw,			//destw
					scr_lh,			//desth
					fromx,			//srcx
					fromy,			//srcy
					fromw,			//srcw
					fromh,			//srch
					0				//is smooth?
				);
			}
			else if (state>0.5){
				//scr_rw=((float)target_canvas->w)*((state-1.0)*2.0);
				scr_rw=target_canvas->w*((state*2.0)-1.0);
				scr_rx=(to->w-scr_rw)/2;
				//ALOGI("Right page: h=%d, y=%d, state=%1.2f", scr_ry, scr_rh, (state*2.0)-1.0);
				libaroma_draw_scale(
					target_canvas,	//dest
					to,				//src
					scr_rx,			//destx
					scr_ry-2,		//desty
					scr_rw,			//destw
					scr_rh,			//desth
					tox,			//srcx
					toy,			//srcy
					tow,			//srcw
					toh,			//srch
					0				//is smooth?
				);

			}
		}
		break;
		case LIBAROMA_ART_SWITCH_ANIMATION_FADE:{
			float bstate=(255.0 * state);
			byte bbstate = (byte) round(bstate);
			libaroma_draw_ex(target_canvas, from, scr_lx, scr_ly, fromx, fromy, fromw, fromh, 0, 0xFF);
			if (bbstate>0) libaroma_draw_ex(target_canvas, to, scr_lx, scr_ly, tox, toy, tow, toh, 1, bbstate);
		}
		break;
		case LIBAROMA_ART_SWITCH_ANIMATION_HORIZ_STRIPES:{
			int stripe_amount=12;
			int stripe_h=toh/stripe_amount;

			//draw below layer
			libaroma_draw_ex(target_canvas, from, scr_lx, scr_ly, fromx, fromy, fromw, fromh, 0, 0xFF);
			//draw stripes
			int i;
		#ifdef LIBAROMA_CONFIG_OPENMP
		#pragma omp parallel for
		#endif
			for (i=0; i<stripe_amount; i++){
				if (i%2 != 0){//left
					//ALOGI("ARTWORKER - Drawing left side %i", i);
					libaroma_draw_ex(target_canvas, to, 0, scr_ry+(stripe_h*i), tox, toy+(stripe_h*i), scr_rw, stripe_h, 0, 0xFF);
				}
				else {//right
					//ALOGI("ARTWORKER - Drawing right side %i", i);
					libaroma_draw_ex(target_canvas, to, scr_rx, scr_ry+(stripe_h*i), scr_rx+tox, toy+(stripe_h*i), scr_rw, stripe_h, 0, 0xFF);
				}
			}
		}
		break;
		case LIBAROMA_ART_SWITCH_ANIMATION_STACKOVER:
		case LIBAROMA_ART_SWITCH_ANIMATION_CLEAN:{
			libaroma_draw_ex(target_canvas, from, scr_lx, scr_ly, fromx, fromy, scr_lw, fromh, 0, 0xFF);
			if (animation==LIBAROMA_ART_SWITCH_ANIMATION_CLEAN) tox += scr_lw;
			libaroma_draw_ex(target_canvas, to, scr_rx, scr_ry, tox, toy, scr_rw, toh, 0, 0xFF);
		}
		break;
		case LIBAROMA_ART_SWITCH_ANIMATION_REVEAL:{
			float scr_ratio;
			scr_ratio=((float)target_canvas->h/(float)target_canvas->w);
			scr_lh=scr_lw*scr_ratio;
			scr_ly=(target_canvas->h-scr_lh)/2;
			scr_rh=scr_rw*scr_ratio;
			scr_rx=(target_canvas->w-scr_rw)/2;
			scr_ry=(target_canvas->h-scr_rh)/2;

			libaroma_draw_ex(target_canvas, from, 0, 0, fromx, fromy, fromw, fromh, 0, 0xFF);
			libaroma_draw_ex(
				target_canvas,	//dest
				to,				//src
				scr_rx,			//destx
				scr_ry,			//desty
				tox+scr_rx,		//srcx
				scr_ry,			//srcy
				scr_rw,			//srcw
				scr_rh,			//srch
				0,				//usealpha
				0xff			//opacity
			);
		}
		break;
		case LIBAROMA_ART_SWITCH_ANIMATION_SLIDE:{
			scr_lx=-(fromw-scr_lw);
			scr_rx=scr_lx+fromw;
			libaroma_draw_ex(target_canvas, from, scr_lx, scr_ly, fromx, fromy, fromw, fromh, 0, 0xFF);
			libaroma_draw_ex(target_canvas, to, scr_rx, scr_ry, tox, toy, tow, toh, 0, 0xFF);
		}
		break;
		case LIBAROMA_ART_SWITCH_ANIMATION_CIRCLE:{
			int bigger=MAX(target_canvas->h, target_canvas->w);
			int sz;
			sz=((bigger*1.5)*state);
			libaroma_draw_ex(target_canvas, from, scr_lx, scr_ly, fromx, fromy, fromw, fromh, 0, 0xFF);
			libaroma_draw_mask_circle(target_canvas, to, target_canvas->w/2, target_canvas->h/2, tox+(tow/2), toy+(toh/2), sz, 0xFF);
		}
		break;
		/*
		case LIBAROMA_ART_SWITCH_ANIMATION_:{

		}
		break;
		*/
	}
	return target_canvas;
} /* End of libaroma_art_draw_switch_animation */


byte libaroma_ani_win_to_art(byte animation){
	switch (animation){
		case LIBAROMA_WINDOW_SHOW_ANIMATION_SLIDE_BOTTOM:
		case LIBAROMA_WINDOW_SHOW_ANIMATION_SLIDE_LEFT:
		case LIBAROMA_WINDOW_SHOW_ANIMATION_SLIDE_RIGHT:
		case LIBAROMA_WINDOW_SHOW_ANIMATION_SLIDE_TOP:
			return LIBAROMA_ART_SWITCH_ANIMATION_SLIDE;
		case LIBAROMA_WINDOW_SHOW_ANIMATION_SWAP_LEFT:
		case LIBAROMA_WINDOW_SHOW_ANIMATION_SWAP_RIGHT:
			return LIBAROMA_ART_SWITCH_ANIMATION_SWAP;
		case LIBAROMA_WINDOW_SHOW_ANIMATION_PAGE_BOTTOM:
		case LIBAROMA_WINDOW_SHOW_ANIMATION_PAGE_LEFT:
		case LIBAROMA_WINDOW_SHOW_ANIMATION_PAGE_RIGHT:
		case LIBAROMA_WINDOW_SHOW_ANIMATION_PAGE_TOP:
			return LIBAROMA_ART_SWITCH_ANIMATION_STACKOVER;
		case LIBAROMA_WINDOW_SHOW_ANIMATION_CIRCLE:
			return LIBAROMA_ART_SWITCH_ANIMATION_CIRCLE;
		case LIBAROMA_WINDOW_SHOW_ANIMATION_FADE:
			return LIBAROMA_ART_SWITCH_ANIMATION_FADE;
		case LIBAROMA_WINDOW_SHOW_ANIMATION_STACKIN:
		case LIBAROMA_WINDOW_SHOW_ANIMATION_STACKOUT:
			return LIBAROMA_ART_SWITCH_ANIMATION_STACKIN;
	}
	return LIBAROMA_WINDOW_SHOW_ANIMATION_FADE;
}

/*
 * Function		: libaroma_animation_random
 * Return Value: byte
 * Descriptions: get random animation type
 */
byte libaroma_animation_random(){
	//return random value between first and last animation numbers
	return (byte)libaroma_random_range(1, 11);
}

#ifdef __cplusplus
}
#endif
#endif /* __libaroma_artworker_c__ */

