# Build the library
Thanks to the Makefile and config.txt files, libaroma now supports not only customizable and faster building, it also allows to be built either as a static, shared or both types of library.
## Prerequisites
You will obviously need some compiler or build environment. As this is not a beginers guide, I will supose that you already have it and it works.
You must have installed the following libraries:
- zlib (used for deflate decompression & is a dependency for libpng/freetype)
- libpng (used for PNG reading/writing)
- libfreetype (used for text rendering)
- libpthread (used for ui/message threading)  

They may have additional dependencies according to how they were compiled (most times they don't), but let's supose they are already installed.
## Optional libraries
There are some optional libraries you can install to support more features:
- libomp or libgomp (OpenMP, allows parallel drawing = it's generally faster)
- libjpeg or libjpeg-turbo (adds jpeg reading support)
- libsdl or libsdl2 (must set `LIBAROMA_PLATFORM` accordingly in config.txt)  

Again, they may have additional dependencies.
## Mandatory configuration
Before building the library, you must set some configurations in the config.txt file.
`LIBAROMA_PLATFORM` must be set to the target platform libaroma is being built for, right now the only supported platforms are:
- linux
- sdl
- sdl2  

There are more options you can set (see [CONFIGURE.md](CONFIGURE.md)) according to your needs, but if you just want to build the library you don't need any extra options.
## Let's build
After making sure that `LIBAROMA_PLATFORM` is set to your target platform, you can build the library. Just use the follwing command:  
`make`  
If everything goes well, you must get the `libaroma.so` file inside the out folder.  
Let's build an example application:  
`make test`  
If it built correctly, you can see the resulting program inside the out folder.
## Building a static library
Note: this isn't recommended when targetting SDL platforms.
If you want a static build of the library, you can build it with the following command:  
`make static`  
To install it, you must use the following command:  
`sudo make install-static`  
In order to statically link to it, you must also link to the dependencies, which currently are the following: (they **must** be in this order)  
`-lfreetype -lpng -lz -fopenmp -lrt -lpthread -lm -lstdc++`  
With any extra dependencies (the ones you would put at LIBAROMA_LIBDEPENDS) before `-fopenmp`.  
Note: you can omit the `-fopenmp` if you disabled LIBAROMA_FEATURE_OPENMP.
# Install the library
Libaroma now can be installed on the system path to provide easier access from any project you have.  
Just make sure that you built the library previously, and using the following command:  
`sudo make install`  
it will be installed in the default path (/usr) or any custom provided path, either via the PREFIX/DESTIR environment variables or the config file by setting LIBAROMA_PREFIX to any path.  
Note that you don't need superuser (root) access to install it if you choose another path and you have permissions to write there.
## Uninstall the library
In case you need to remove the library from your system, jut use:  
`sudo make uninstall`  
Note that you don't need superuser (root) access if library was installed in a custom path and you have access there.
# Tested build environments & quirks
Debian 11.5 x64, both distro-provided GCC 10.2.5 & Clang 11.0.1
- Debian's provided libfreetype-dev depends on libbrotli-dev, and you must add the following flags to `LIBAROMA_LIBDEPENDS` in config.txt:  
  `-lbrotlidec -lbrotlicommon`
- Default GCC can build both shared & static lib/test without issues
- Default Clang fails to find omp library when static linking, if OpenMP is enabled in config.  

Alpine 3.17 armhf, both Musl GCC 12.1.1 & Clang 15.0.6
- Alpine's freetype-dev depends on the same as Debian's build, but adds one more prerequisite: bzip2-dev. You also have to add it at the end of the depends list:
  `-lbrotlidec -lbrotlicommon -lbz2`
- Default GCC can build both shared & static lib/test without issues
- Default Clang also fails to find omp library when static linking if needed.  
  Also, throws `plugin needed to handle lto object` on some freetype depends.  

Windows 10 x64, MSYS2 MinGW64 (GCC 12.2.0)
- Works for shared library
- Could not test static build because SDL is the only platform that works on Windows and static linking SDL never works.  

Windows 7 x86, Cygwin MinGW32 (GCC 11.3.0)
- Same as Windows 10 x64, MSYS2 MinGW64
