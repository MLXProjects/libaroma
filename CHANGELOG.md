libaroma 1.1.0-230111:  
changes: 
- huge file layout refactor
- Now buildable with Make
- some documentation added
- fixed mem corruptions/undefined behavior in xml utility
- added line number to xml tags
- fixed lots of pointer-to-int & viceversa conversions in text and engines
- libaroma_random_range now returns int as it should
- fixed missing/wrong returns in ctl_clock & listitem_text
- fixed no-mmap file loading
- reworked canvas shmem open/create
- minzip now uses libaroma_file for zip opening
- combined SDL and SDL2 platforms into a generic one
- refactored SDL2 platform (fixes out-of-vram issue)
- implemented simple pseudo-random number generator (replaces rand())
- updated x86 (SSSE3) color optimizations

removed: 
- custom build systems (win32/linux)
- platform-specific (accelerated) memset, had trouble with some compilers
- old, unmantained/untested platforms like qnx & rpi
- DRM graphics support (temporarily, must be reimplemented decently)
- 99% of aroma runtime (the runtime monitor remains)
- selinux capabilites from minzip
- lots of unused variables

issues found:
- vmware's vmwgfx framebuffer gives an invalid smem_len size, and on writing it just hangs the entire program.
- showing detailed build information looks screwed using certain Clang versions, reproducible with v11.0.1-2
