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
 * Filename		: fb_qcom.c
 * Description : linux framebuffer driver for qcom msmfb4X & mdssfb
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 26/01/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_linux_fb_qcom_driver_c__
#define __libaroma_linux_fb_qcom_driver_c__

#include <aroma_internal.h>
#include <errno.h>
#include "fb.h"
#include "msm_mdp.h"

#ifdef __cplusplus
extern "C" {
#endif
/*
 * Function		: QCOMFB_init
 * Return Value: byte
 * Descriptions: init qcom overlay driver
 */
byte QCOMFB_init(LIBAROMA_FBP me){
	if (me == NULL) {
		return 0;
	}
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;

	/* check id */
	byte isqcom=QCOMFB_check_id(mi);
	if (!isqcom){
		return 0;
	}

	/* allocating qcom internal data */
	mi->backend = calloc(sizeof(QCOMFB_INTERNAL),1);
	if (!mi->backend){
		ALOGW("QCOMFB_init cannot allocating qcom internal data");
		return 0;
	}
	QCOMFB_INTERNALP qi = (QCOMFB_INTERNALP) mi->backend;
	qi->type = LINUXFBDR_BACKEND_QCOM;
	qi->id = isqcom;

	qi->dbuf=1;
	me->double_buffer=1;
	ALOGV("QCOMFB_init got qcom. #%i - %i", qi->id, qi->dbuf);

	/* open ion device */
	qi->ionfd = open("/dev/ion", O_RDWR|O_SYNC);
	if (qi->ionfd<0) {
		ALOGV("QCOMFB_init cannot open /dev/ion");

		ALOGV("Error no is : %d\n", errno);
		ALOGV("Error description is : %s\n",strerror(errno));

		free(qi);
		mi->backend=NULL;
		return 0;
	}

	/* calculate size */
	mi->line	= me->w;
	if (qi->id){
		mi->depth = 16;
		mi->pixsz = 2;
	}
	else{
		mi->depth		 = mi->var.bits_per_pixel;	 /* color depth */
		mi->pixsz		 = mi->depth >> 3;					 /* pixel size per byte */
		if (mi->var.transp.offset){
			LINUXFBDR_setrgbpos(me,16,8,0);
		}
		else{
			LINUXFBDR_setrgbpos(me,0,8,16);
		}
	}

	mi->fb_sz = mi->line * me->h * mi->pixsz * (qi->dbuf?2:1);
	mi->stride= (mi->line - me->w) * mi->pixsz;
	mi->line = mi->line * mi->pixsz;

	ALOGV("QCOMFB_init info (l:%i, s:%i, sz:%i, d:%i, p:%i)",
		mi->line,
		mi->stride,
		mi->fb_sz,
		mi->depth,
		mi->pixsz
	);

	/* allocation ion data */
	struct ion_allocation_data ion_alloc;
	ion_alloc.flags = 0;
	ion_alloc.len = mi->fb_sz;
	ion_alloc.align = sysconf(_SC_PAGESIZE);
	ion_alloc.heap_mask =
			ION_HEAP(ION_IOMMU_HEAP_ID) |
			ION_HEAP(ION_SYSTEM_CONTIG_HEAP_ID);
	if(ioctl(qi->ionfd, ION_IOC_ALLOC, &ion_alloc)){
		ALOGV("QCOMFB_init ION_IOC_ALLOC Failed");
		close(qi->ionfd);
		free(qi);
		mi->backend=NULL;
		return 0;
	}

	/* prepare mmap ion data */
	struct ion_fd_data ion_data;
	ion_data.handle = ion_alloc.handle;
	qi->handle = ion_alloc.handle;
	if (ioctl(qi->ionfd, ION_IOC_MAP, &ion_data)) {
		ALOGV("QCOMFB_init ION_IOC_MAP Failed");
		QCOMFB_release(me);
		return 0;
	}

	/* set memfd */
	qi->memfd = ion_data.fd;

	/* mmap */
	mi->buffer = (voidp)
		mmap(NULL,mi->fb_sz,PROT_READ|PROT_WRITE,MAP_SHARED,qi->memfd,0);
	if (!mi->buffer) {
		ALOGW("QCOMFB_init mi->buffer MAP_FAILED");
		QCOMFB_release(me);
		return 0;
	}

	/* split display */
	QCOMFB_split_display(me);

	/* request overlays */
	qi->overlay_lid=MSMFB_NEW_REQUEST;
	qi->overlay_rid=MSMFB_NEW_REQUEST;
	if (!QCOMFB_allocate_overlays(me)){
		ALOGI("QCOMFB_init cannot allocate overlays");
		QCOMFB_release(me);
		return 0;
	}

	/* set overlay player */
	qi->commit_type = 0;
	memset(&qi->overlay, 0, sizeof(struct msmfb_overlay_data));
	qi->overlay.data.id = qi->memfd;
	qi->overlay.data.flags = 0;
	qi->overlay.data.offset = 0;
	qi->overlay.data.memory_id = qi->memfd;

	/* set commiter data */
	memset(&qi->commiter, 0, sizeof(struct mdp_display_commit));
	qi->commiter.flags = MDP_DISPLAY_COMMIT_OVERLAY;
	qi->commiter.wait_for_finish = 1;

	/* pos
	qi->commiter.l_roi.x = 0;
	qi->commiter.l_roi.y = 0;
	qi->commiter.l_roi.w = me->w;
	qi->commiter.l_roi.h = me->h;
	qi->commiter.r_roi.x = 0;
	qi->commiter.r_roi.y = 0;
	qi->commiter.r_roi.w = me->w;
	qi->commiter.r_roi.h = me->h;
	*/

	/* swap buffer & commit now */
	QCOMFB_flush(me);


	/* successed */
	ALOGI("QCOMFB_init qcom overlay driver successfull (Type:%i)",
		qi->id);
	return 1;
} /* End of QCOMFB_init */

/*
 * Function		: QCOMFB_release
 * Return Value: void
 * Descriptions: release qcom overlay driver
 */
void QCOMFB_release(LIBAROMA_FBP me){
	if (me == NULL) {
		return;
	}
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
	QCOMFB_INTERNALP qi = (QCOMFB_INTERNALP) mi->backend;
	if (qi){
		/* unset overlays */
		if (qi->overlay_lid != MSMFB_NEW_REQUEST) {
			ioctl(mi->fb, MSMFB_OVERLAY_UNSET, &qi->overlay_lid);
		}
		if (qi->overlay_rid!=MSMFB_NEW_REQUEST){
			ioctl(mi->fb, MSMFB_OVERLAY_UNSET, &qi->overlay_rid);
		}

		/* flush close */
		QCOMFB_flush(me);

		if (mi->buffer){
			munmap(mi->buffer, mi->fb_sz);
			mi->buffer=NULL;
		}
		if (qi->ionfd>=0) {
			ioctl(qi->ionfd, ION_IOC_FREE, &qi->handle);
		}
		if (qi->memfd>=0){
			close(qi->memfd);
		}
		if (qi->ionfd>=0){
			close(qi->ionfd);
		}

		/* free qcom internal data */
		free(qi);
		mi->backend=NULL;
	}
} /* End of QCOMFB_release */

/*
 * Function		: QCOMFB_check_id
 * Return Value: byte
 * Descriptions: check framebuffer id
 */
byte QCOMFB_check_id(LINUXFBDR_INTERNALP mi){
	/* check ion device */
	byte overlay_supported = 0;
	byte isMDP5 = 0;
	if(!strncmp(mi->fix.id, "msmfb", strlen("msmfb"))) {
		char str_ver[4];
		memcpy(str_ver, mi->fix.id + strlen("msmfb"), 3);
		str_ver[3] = '\0';
		int mdp_version = atoi(str_ver);
		ALOGV("QCOMFB_init ver: %i",mdp_version);
		if (mdp_version>=400) {
			overlay_supported = 1;
		}
		else if (mdp_version>=44){
			return 1;
		}
	}
	else if (!strncmp(mi->fix.id, "mdssfb", strlen("mdssfb"))) {
		overlay_supported = 1; /* non support for now */
		isMDP5 = 1;
	}
	if (!overlay_supported){
		return 0;
	}
	if (overlay_supported==3){
		return 1;
	}
	return (isMDP5?4:5);
} /* End of QCOMFB_check_id */

/*
 * Function		: QCOMFB_split_display
 * Return Value: void
 * Descriptions: get split display info
 */
void QCOMFB_split_display(LIBAROMA_FBP me){
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
	QCOMFB_INTERNALP qi = (QCOMFB_INTERNALP) mi->backend;
	char split[64] = {0};
	FILE* fp = fopen("/sys/class/graphics/fb0/msm_fb_split", "r");
	if (fp) {
		if(fread(split, sizeof(char), 64, fp)) {
			qi->split_left = atoi(split);
			char *right = strpbrk(split, " ");
			if (right){
				qi->split_right = atoi(right + 1);
			}
		}
		ALOGV("QCOMFB_split_display msm_fb_split %i,%i",
			qi->split_left,
			qi->split_right
		);
		fclose(fp);
	}

	/* check split display */
	if ((me->w>MAX_DISPLAY_DIM)||(qi->split_right)){
		qi->split=1;
		if (!qi->split_right){
			qi->split_right = me->w / 2;
		}
		if (!qi->split_left){
			qi->split_left = me->w - qi->split_right;
		}
	}
	else{
		qi->split=0;
		qi->split_right=0;
		qi->split_left=me->w;
	}

	/* log */
	ALOGI("QCOMFB_split_display=%i, left=%i, right=%i",
		qi->split,
		qi->split_left,
		qi->split_right
	);
} /* End of QCOMFB_split_display */

/*
 * Function		: QCOMFB_overlay
 * Return Value: int
 * Descriptions: register overlay
 */
int QCOMFB_overlay(LIBAROMA_FBP me, LINUXFBDR_INTERNALP mi,
		int sl, int sr, int dl, int dr, byte isright){
	QCOMFB_INTERNALP qi = (QCOMFB_INTERNALP) mi->backend;
	int i;
	ALOGV("QCOMFB_overlay req(%i,%i,%i,%i)",sl,sr,dl,dr);
	for (i=3;i>=0;i--){
		if (qi->id==1){
			struct mdp_overlay_44 ovrl;
			memset(&ovrl, 0 , sizeof (struct mdp_overlay_44));
			if (mi->pixsz==2){
				ovrl.src.format = MDP_RGB_565;
			}
			else{
				if (mi->var.transp.offset){
					ovrl.src.format = MDP_BGRX_8888;
				}
				else{
					ovrl.src.format = MDP_RGBX_8888;
				}
			}
			ovrl.src.width	= me->w;
			ovrl.src.height = me->h;// * (qi->dbuf?2:1);
			ovrl.src_rect.x = sl;
			ovrl.src_rect.y = 0;
			ovrl.src_rect.w = sr;
			ovrl.src_rect.h = me->h;
			ovrl.dst_rect.x = dl;
			ovrl.dst_rect.y = 0;
			ovrl.dst_rect.w = dr;
			ovrl.dst_rect.h = me->h;
			ovrl.alpha = 0xFF;
			ovrl.transp_mask = MDP_TRANSP_NOP;
			ovrl.id = MSMFB_NEW_REQUEST;
			ovrl.z_order=i;
			if (isright){
				ovrl.flags = MDSS_MDP_RIGHT_MIXER;
			}
			if (ioctl(mi->fb, MSMFB_OVERLAY_SET_44, &ovrl)==0){
				if (((int) ovrl.id)!=MSMFB_NEW_REQUEST){
					ALOGV("QCOMFB_overlay[44](#%i, z_order:%i)",ovrl.id,i);
					return ovrl.id;
				}
			}
		}
		else{
			struct mdp_overlay ovrl;
			memset(&ovrl, 0 , sizeof (struct mdp_overlay));
			if (mi->pixsz==2){
				ovrl.src.format = MDP_RGB_565;
			}
			else{
				if (mi->var.transp.offset){
					ovrl.src.format = MDP_BGRX_8888;
				}
				else{
					ovrl.src.format = MDP_RGBX_8888;
				}
			}
			ovrl.src.width	= me->w;
			ovrl.src.height = me->h;// * (qi->dbuf?2:1);
			ovrl.src_rect.x = sl;
			ovrl.src_rect.y = 0;
			ovrl.src_rect.w = sr;
			ovrl.src_rect.h = me->h;
			ovrl.dst_rect.x = dl;
			ovrl.dst_rect.y = 0;
			ovrl.dst_rect.w = dr;
			ovrl.dst_rect.h = me->h;
			ovrl.alpha = 0xFF;
			ovrl.transp_mask = MDP_TRANSP_NOP;
			ovrl.id = MSMFB_NEW_REQUEST;
			ovrl.z_order=i;
			if (isright){
				ovrl.flags = MDSS_MDP_RIGHT_MIXER;
			}
			if (ioctl(mi->fb, MSMFB_OVERLAY_SET, &ovrl)==0){
				if (((int) ovrl.id)!=MSMFB_NEW_REQUEST){
					ALOGV("QCOMFB_overlay(#%i, z_order:%i)",ovrl.id,i);
					return ovrl.id;
				}
			}
		}
	}
	return MSMFB_NEW_REQUEST;
} /* End of QCOMFB_overlay */

/*
 * Function		: QCOMFB_allocate_overlays
 * Return Value: byte
 * Descriptions: allocate mdp overlays
 */
byte QCOMFB_allocate_overlays(LIBAROMA_FBP me){
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
	QCOMFB_INTERNALP qi = (QCOMFB_INTERNALP) mi->backend;
	qi->overlay_lid = QCOMFB_overlay(me,mi,
		0,qi->split_left,0, qi->split_left, 0);
	if (qi->overlay_lid==MSMFB_NEW_REQUEST){
		ALOGV("QCOMFB_allocate_overlays - alloc left overlay failed");
		return 0;
	}
	if (qi->split){
		qi->overlay_rid = QCOMFB_overlay(me,mi,
			qi->split_left,
			qi->split_right,
			0,
			qi->split_right,
			1
		);

		if (qi->overlay_rid==MSMFB_NEW_REQUEST){
			ALOGV("QCOMFB_allocate_overlays - alloc right overlay failed");
			return 0;
		}
	}
	/* prepare overlay data */
	ALOGI("QCOMFB_allocate_overlays successfull ID=#%i,#%i",
		qi->overlay_lid, qi->overlay_rid);
	return 1;
} /* End of QCOMFB_allocate_overlays */

/*
 * Function		: QCOMFB_start_post
 * Return Value: byte
 * Descriptions: start post
 */
byte QCOMFB_start_post(LIBAROMA_FBP me){
	if (me == NULL) {
		return 0;
	}
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
	libaroma_mutex_lock(mi->mutex);
	return 1;
}

/*
 * Function		: QCOMFB_end_post
 * Return Value: byte
 * Descriptions: end post
 */
byte QCOMFB_end_post(LIBAROMA_FBP me){
	if (me == NULL) {
		return 0;
	}
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
	QCOMFB_INTERNALP qi = (QCOMFB_INTERNALP) mi->backend;
	/* display frame */
	if (qi->dbuf){
		int doffset = qi->yoffset*mi->line;
		qi->overlay.data.offset = doffset;
		qi->overlay.id = qi->overlay_lid;
		ioctl(mi->fb, MSMFB_OVERLAY_PLAY, &qi->overlay);
		if (qi->split){
			qi->overlay.data.offset = doffset;
			qi->overlay.id = qi->overlay_rid;
			ioctl(mi->fb, MSMFB_OVERLAY_PLAY, &qi->overlay);
		}
	}
	else{
		qi->overlay.data.offset = 0;
		qi->overlay.id = qi->overlay_lid;
		ioctl(mi->fb, MSMFB_OVERLAY_PLAY, &qi->overlay);
		if (qi->split){
			qi->overlay.data.offset = 0;
			qi->overlay.id = qi->overlay_rid;
			ioctl(mi->fb, MSMFB_OVERLAY_PLAY, &qi->overlay);
		}
	}
	QCOMFB_flush(me);
	libaroma_mutex_unlock(mi->mutex);
	return 1;
}

/*
 * Function		: QCOMFB_post
 * Return Value: byte
 * Descriptions: post
 */
byte QCOMFB_post(
	LIBAROMA_FBP me, wordp __restrict src,
	int dx, int dy, int dw, int dh,
	int sx, int sy, int sw, int sh
	){
	if (me == NULL) {
		return 0;
	}
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;

	int sstride = (sw - dw) * 2;
	int dstride = (me->w - dw) * mi->pixsz;
	wordp copy_src = (wordp) (src + (sw * sy) + sx);
	bytep dst_addr = (((bytep) mi->current_buffer)+(mi->line*dy)+(dx*mi->pixsz));
	if (mi->pixsz==2){
		libaroma_blt_align16(
			(wordp) dst_addr,
			copy_src,
			dw, dh,
			dstride,
			sstride
		);
	}
	else{
		libaroma_blt_align_to32_pos(
			(dwordp) dst_addr,
			copy_src,
			dw, dh,
			dstride,
			sstride,
			mi->rgb_pos
		);
	}
	return 1;
}

/*
 * Function		: QCOMFB_swap_buffer
 * Return Value: void
 * Descriptions: qcom swap back buffer
 */
void QCOMFB_swap_buffer(LIBAROMA_FBP me){
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
	QCOMFB_INTERNALP qi = (QCOMFB_INTERNALP) mi->backend;
	if (qi->dbuf){
		if (qi->yoffset==0){
			qi->yoffset=me->h;
		}
		else{
			qi->yoffset=0;
		}
		mi->current_buffer = ((bytep) mi->buffer) + (qi->yoffset * mi->line);
	}
	else{
		qi->yoffset=0;
		mi->current_buffer = mi->buffer;
	}
} /* End of QCOMFB_swap_buffer */

/*
 * Function		: QCOMFB_flush
 * Return Value: void
 * Descriptions: flush overlay update
 */
void QCOMFB_flush(LIBAROMA_FBP me){
	LINUXFBDR_INTERNALP mi = (LINUXFBDR_INTERNALP) me->internal;
	QCOMFB_INTERNALP qi = (QCOMFB_INTERNALP) mi->backend;
	QCOMFB_swap_buffer(me);

	int res=0;
	if (qi->commit_type==1){
		res=ioctl(mi->fb, MSMFB_DISPLAY_COMMIT_44, &qi->commiter);
	}
	else if (qi->commit_type==2){
		res=ioctl(mi->fb, MSMFB_DISPLAY_COMMIT_NON_NOTE4, &qi->commiter);
	}
	else if (qi->commit_type==3){
		res=ioctl(mi->fb, MSMFB_DISPLAY_COMMIT, &qi->commiter);
	}
	else if (qi->commit_type==4){
		mi->var.activate = FB_ACTIVATE_VBL;
		if (ioctl(mi->fb, FBIOPAN_DISPLAY, &mi->var)!=0){
			ioctl(mi->fb, FBIOPUT_VSCREENINFO, &mi->var);
		}
	}
	else if (qi->commit_type==0){
		if (ioctl(mi->fb,MSMFB_DISPLAY_COMMIT_44,&qi->commiter)!=0){
			if (ioctl(mi->fb,MSMFB_DISPLAY_COMMIT_NON_NOTE4,&qi->commiter)!=0){
				if (ioctl(mi->fb, MSMFB_DISPLAY_COMMIT,&qi->commiter)!=0){
					mi->var.activate =FB_ACTIVATE_VBL;
					if (ioctl(mi->fb, FBIOPAN_DISPLAY, &mi->var)!=0){
						ioctl(mi->fb, FBIOPUT_VSCREENINFO, &mi->var);
					}
					qi->commit_type=4;
					ALOGV("QCOMFB commit type fallback fb");
				}
				else{
					qi->commit_type=3;
					ALOGV("QCOMFB commit type new msm_mdp note4");
				}
			}
			else{
				qi->commit_type=2;
				ALOGV("QCOMFB commit type new msm_mdp");
			}
		}
		else{
			qi->commit_type=1;
			ALOGV("QCOMFB commit type old msm_mdp v44");
		}
	}

	if (res){
		// printf("%i",res); fflush(stdout);
	}
} /* End of QCOMFB_flush */

#ifdef __cplusplus
}
#endif
#endif /* __libaroma_linux_fb_qcom_driver_c__ */
