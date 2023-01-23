# Libaroma
an embedded UI toolkit
## What is this?
It's a C library which provides graphics output, input management, utilities for drawing and a window system with controls that allows to create an entire user interface with few code lines.
## Features
- input management
- graphics output
- basic window manager (one window rendered at a time)
- hardware acceleration whenever possible (ARM NEON, x86 SSE)
- simple to use and understand
- most parts have automatic allocation/free (you only need to release the library)
## Compatibility
For now it was tested on the following platforms:
- Linux (either direct framebuffer/input or SDL window)
- Windows (using SDL window)
## How to use it?
First build and install the library, [refer to the BUILDING.md file](BUILDING.md). 
There are some examples already written, but the following code should be enough to initialize the library, create an empty window and handle the input:

```
#include <aroma.h>
int main(int argc, char **argv){
  if (!libaroma_start()){
      printf("libaroma start failed\n");
      return 0;
  }
  LIBAROMA_WINDOWP win = libaroma_window(NULL, 0, 0, 
			LIBAROMA_SIZE_FULL, LIBAROMA_SIZE_FULL);
  if (win==NULL){
      printf("libaroma window failed\n");
      libaroma_end();
      return 0;
  }
  libaroma_window_show(win);
  LIBAROMA_MSG msg;
  do {
      libaroma_window_pool(win, &msg);
      if (msg.msg==LIBAROMA_MSG_EXIT){
          win->onpool=0;
      }
      else if (msg.msg==LIBAROMA_MSG_TOUCH &&
		msg.state==LIBAROMA_HID_EV_STATE_UP){
          libaroma_msg_post(LIBAROMA_MSG_EXIT, 0, 0, 0, 0, NULL);
      }
      else printf("msg=% state=%d, key=%d, x=%d, y=%d, data=%p\n", 
                  msg.msg, msg.state, msg.key, msg.x, msg.y, msg.d);
  } while(win->onpool);
  libaroma_window_free(win);
  libaroma_end();
}
```
After this, just compile the program as you would with any project, for example:  
`gcc main.c -o test -laroma`  
Run it and should show an empty screen/window that closes after releasing the mouse left click.
## TODO
- fix scroll control relying on screen size for fling velocity 
- pager inside a fragment may give black control at creation
- re-implement DRM on Linux
- multiple windows & z order
- LUA scripting engine?
## Disclaimer
This library is under heavy work in progress, while there are no planned API changes that break compatibility it may happen if needed (e.g. in case of redundant features removed or safety issues found).
The following libraries were embedded to easily implement features:
- cJSON ([thanks Dave Gamble](https://github.com/DaveGamble/cJSON))
- Mini-XML ([thanks [Michael Sweet](https://github.com/michaelrsweet/mxml))
- nanoSVG ([thanks Mikko Mononen](https://github.com/memononen/nanosvg))
- MinZIP ([thanks Google](https://android.googlesource.com/platform/bootable/recovery.git/+/refs/heads/nougat-release/minzip/))
## License
Libaroma is licensed under the Apache 2.0 license ([see LICENSE.MD file](LICENSE.md))
The embedded libraries listed above have their own licenses:
- cJSON is covered by the MIT license
- Mini-XML is covered by the Apache 2.0 license
- nanoSVG is covered by the zlib license
- MinZIP isn't a library per se, but it's code is covered by the Apache 2.0 license
