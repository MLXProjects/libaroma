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
 * Filename		: artworker.h
 * Description : artworker
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 25/03/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_aroma_h__
	#error "Include <aroma.h> instead."
#endif
#ifndef __libaroma_artworker_h__
#define __libaroma_artworker_h__

#define LIBAROMA_ART_SWITCH_ANIMATION_SLIDE			1
#define LIBAROMA_ART_SWITCH_ANIMATION_SCALE			2
#define LIBAROMA_ART_SWITCH_ANIMATION_STACKIN		3
#define LIBAROMA_ART_SWITCH_ANIMATION_STACKOVER		4
#define LIBAROMA_ART_SWITCH_ANIMATION_REVEAL		5
#define LIBAROMA_ART_SWITCH_ANIMATION_STRETCH		6
#define LIBAROMA_ART_SWITCH_ANIMATION_CLEAN			7
#define LIBAROMA_ART_SWITCH_ANIMATION_CIRCLE		8
#define LIBAROMA_ART_SWITCH_ANIMATION_PAGE			9
#define LIBAROMA_ART_SWITCH_ANIMATION_FADE			10
#define LIBAROMA_ART_SWITCH_ANIMATION_SWAP			11
#define LIBAROMA_ART_SWITCH_ANIMATION_STRETCH_IN	12
#define LIBAROMA_ART_SWITCH_ANIMATION_HORIZ_STRIPES	13

/* TODO: switch animation sides - use in shifting animations (default to horizontal)
#define LIBAROMA_ART_SWITCH_TOP			0x2
#define LIBAROMA_ART_SWITCH_LEFT		0x3
#define LIBAROMA_ART_SWITCH_BOTTOM		0x4
#define LIBAROMA_ART_SWITCH_RIGHT		0x5
#define LIBAROMA_ART_SWITCH_VERTICAL	0x6
#define LIBAROMA_ART_SWITCH_HORIZONTAL	0x8 */

/*
 * Variable		: libaroma_art_svg_logo
 * Type			: char[]
 * Descriptions: aroma logo - svg format
 */
extern char libaroma_art_svg_logo[];

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
	float aliasing);

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
	float state);

/*
 * Function		: libaroma_ani_win_to_art
 * Return Value: byte
 * Descriptions: translate window show animation to art switch
 */
byte libaroma_ani_win_to_art(byte animation);

/*
 * Function		: libaroma_animation_random
 * Return Value: byte
 * Descriptions: get random animation type
 */
byte libaroma_animation_random();

#endif /* __libaroma_artworker_h__ */
