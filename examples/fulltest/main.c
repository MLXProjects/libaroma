#include <aroma.h>

int main(int argc, char **argv){
	printf("%s: starting (%d params - random number=%d)\n", argv[0], argc, libaroma_random());
	//libaroma_config()->runtime_monitor=LIBAROMA_START_SAFE;
	if (!libaroma_start()){
		printf("libaroma start failed\n");
		return 0;
	}
	printf("libaroma started, creating window\n");
	LIBAROMA_WINDOWP win = libaroma_window(NULL, 0, 0, LIBAROMA_SIZE_FULL, LIBAROMA_SIZE_FULL);
	if (win==NULL){
		printf("libaroma window failed\n");
		libaroma_end();
		return 0;
	}
	libaroma_ctl_progress(win, 0, (libaroma_width_dp()/2)-24, (libaroma_height_dp()/2)-24, 48, 48,
							LIBAROMA_CTL_PROGRESS_CIRCULAR|LIBAROMA_CTL_PROGRESS_INDETERMINATE, 0, 0);
	printf("window created, showing\n");
	libaroma_window_show(win);
	LIBAROMA_MSG msg;
	do {
		printf("waiting for input\n");
		libaroma_window_pool(win, &msg);
		if (msg.msg==LIBAROMA_MSG_EXIT){
			win->onpool=0;
		}
		else if (msg.msg==LIBAROMA_MSG_TOUCH && msg.state==LIBAROMA_HID_EV_STATE_UP){
			libaroma_msg_post(LIBAROMA_MSG_EXIT, 0, 0, 0, 0, NULL);
		}
		else printf("msg=%d state=%d, key=%d, x=%d, y=%d, data=%p\n", 
					msg.msg, msg.state, msg.key, msg.x, msg.y, msg.d);
	} while(win->onpool);
	libaroma_window_free(win);
	libaroma_end();
}