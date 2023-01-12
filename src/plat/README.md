# Platform sources
This folder contains all platform targets supported by libaroma.
You can add here any custom platfoms you've written/ported, but make sure to add any library dependencies they might have at libaroma's Makefile too.
## How does a "platform" work?
Platforms are quite simple, they just have three purposes:
- provide a buffer to use as graphics output
- provide an input manager for request events to
- define the platform callbacks to be used by libaroma.
You can do crazy things like using a file as a graphics buffer and another as input pipe if you know how :P ([see my attempt to do such a thing](https://www.youtube.com/watch?v=Yx1Dsi56-xE))
## Some example
We will examine the simplest platform we have at hand: SDL.
### Platform features
Platforms must set what system features are available, in order to disable libaroma features unsupported by the target system. These are:
- mmap support
- shared memory
- file descriptors
- file delete wrapper
- sleep() wrapper (must take milliseconds as input)
mmap, shared memory and file descriptors are optional, but file delete and sleep must be set (file delete wrapper can be set to an empty macro)
Another mandatory wrapper is for threading, mutexes and conditionals support, in case of SDL and Linux it uses pthread.
### Initialization
Libaroma starts by calling the initializer function `libaroma_fb_driver_init` to setup the frmebuffer instance (LIBAROMA_FB) with platform specific callbacks.
Looking at src/plat/sdl/fb.c you can see it's just a wrapper which calls SDLFBDR_init, which is the real framebuffer initializer.
SDLFBDR_init does the following steps:
- creates a SDL window
- gets a memory buffer from that window
- defines the screen size
- sets the callbacks to use when updating, flushing or releasing the screen
After this, next initialized platform-dependant subsystem is the input driver. To do this, the other initializer function `libaroma_hid_driver_init` is called; again it's just a wrapper to SDLHIDRV_init.
SDLHIDRV_init, just like the framebuffer initializer, sets the callbacks to be used by libaroma, in this case they're just two: 
- getinput
- release
Input driver is "simpler" because it just doesn't need much else than, well, getting input from the actual driver; and don't forget we're using SDL as example, which saves us the hassle to (for example) open /dev/input and parse it's devices like we would do in Linux.
### Runtime usage
For input, while the window manager is running it will be signaled by the input thread (which keeps listening for events using the getinput callback set at startup).
For framebuffer, while the library itself is running it will allow the user to post content on the screen and flush it (see libaroma_fb_post & libaroma_fb_sync).
### Releasing
When you ask libaroma to exit (calling libaroma_end()) it will use the callbacks set by the fb/hid drivers, so they handle the actual framebuffer/input releasing.
In SDL's case, input releasing will just stop listening for events and framebuffer releasing will actually call SDL_Quit which ends SDL library.