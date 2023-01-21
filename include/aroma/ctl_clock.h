/*
 * ctl_clock.h
 *
 *  Created on: 2016年8月7日
 *      Author: fine
 *  Modified on: 2021/3/21
 *      By: MLX
 */

#ifndef __libaroma_aroma_h__
	#error "Include <aroma.h> instead."
#endif

#ifndef __libaroma_ctl_clock_h__
#define __libaroma_ctl_clock_h__
#define LIBAROMA_CTL_CLOCK_NOBORDER		0x2		/* disable border drawing */
#define LIBAROMA_CTL_CLOCK_ALPHA		0x4		/* enable alpha blended indicators */
#define LIBAROMA_CTL_CLOCK_RIPPLE		0x8		/* ripple on touch */
#define LIBAROMA_CTL_CLOCK_FREEBG		0x16	/* free previous bg */

#define LIBAROMA_CTL_CLOCK_COLOR_HOUR	0x0
#define LIBAROMA_CTL_CLOCK_COLOR_MINUTE	0x1
#define LIBAROMA_CTL_CLOCK_COLOR_SECOND	0x2
#define LIBAROMA_CTL_CLOCK_COLOR_BORDER	0x3
#define LIBAROMA_CTL_CLOCK_COLOR_CENTER	0x4

LIBAROMA_CONTROLP libaroma_ctl_clock(LIBAROMA_WINDOWP win, dword id, int x,
		int y, int w, int h, LIBAROMA_CANVASP bg, byte flags);

byte libaroma_ctl_clock_setcolor(LIBAROMA_CONTROLP ctl, byte type, word color);

byte libaroma_ctl_clock_setalpha(LIBAROMA_CONTROLP ctl, byte alpha);

#define libaroma_ctl_clock_setbg(ctl, bg) libaroma_ctl_clock_setbg_ex(ctl, bg, 0)
byte libaroma_ctl_clock_setbg_ex(LIBAROMA_CONTROLP ctl, LIBAROMA_CANVASP bg, byte use_orig);

#define libaroma_ctl_clock_setnoborder(ctl) libaroma_ctl_clock_setborder(ctl, 0)
byte libaroma_ctl_clock_setborder(LIBAROMA_CONTROLP ctl, byte enabled);

#endif /* __libaroma_ctl_clock_h__ */
