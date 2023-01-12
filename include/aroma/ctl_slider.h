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
 * Filename		: ctl_slider.h
 * Description : slider control
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 24/11/21 - Author(s): Michael Jauregui
 *
 */
#ifndef __libaroma_aroma_h__
	#error "Include <aroma.h> instead."
#endif
#ifndef __libaroma_ctl_slider_h__
#define __libaroma_ctl_slider_h__

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
);

/*
 * Function		: libaroma_ctl_slider_timing
 * Return Value: byte
 * Descriptions: set slider animation timing
 */
byte libaroma_ctl_slider_timing(
		LIBAROMA_CONTROLP ctl,
		int timing
);

/*
 * Function		: libaroma_ctl_slider_type
 * Return Value: byte
 * Descriptions: set slider type
 */
byte libaroma_ctl_slider_type(
		LIBAROMA_CONTROLP ctl,
		byte type
);

/*
 * Function		: libaroma_ctl_slider_value
 * Return Value: byte
 * Descriptions: set slider value
 */
byte libaroma_ctl_slider_value(
		LIBAROMA_CONTROLP ctl,
		int value
);

/*
 * Function		: libaroma_ctl_slider_max
 * Return Value: byte
 * Descriptions: set slider max value
 */
byte libaroma_ctl_slider_max(
		LIBAROMA_CONTROLP ctl,
		int max
);

/*
 * Function		: libaroma_ctl_slider_set_onchange
 * Return Value: byte
 * Descriptions: set slider update callback
 */
byte libaroma_ctl_slider_set_onchange(
		LIBAROMA_CONTROLP ctl, 
		void *onchange
);

/*
 * Function		: libaroma_ctl_slider_get_timing
 * Return Value: int
 * Descriptions: get slider current animation timing
 */
int libaroma_ctl_slider_get_timing(
		LIBAROMA_CONTROLP ctl
);

/*
 * Function		: libaroma_ctl_slider_get_value
 * Return Value: int
 * Descriptions: get slider current value
 */
int libaroma_ctl_slider_get_value(
		LIBAROMA_CONTROLP ctl
);

/*
 * Function		: libaroma_ctl_slider_get_max
 * Return Value: int
 * Descriptions: get slider max value
 */
int libaroma_ctl_slider_get_max(
		LIBAROMA_CONTROLP ctl
);

/*
 * Function		: libaroma_ctl_slider_get_type
 * Return Value: byte
 * Descriptions: get slider flags
 */
byte libaroma_ctl_slider_get_type(
		LIBAROMA_CONTROLP ctl
);

#endif /* __libaroma_ctl_slider_h__ */
