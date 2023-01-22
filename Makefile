###
# DEFAULT SETTINGS & PATHS
# 
# include settings file first (to override CC, AR, etc.)
include config.txt
# check for must-have variables
ifeq ($(LIBAROMA_PLATFORM),)
$(error LIBAROMA_PLATFORM must be set)
endif #empty

# default build commands
CC := gcc
AR := ar
RANLIB := ranlib
MKDIR := mkdir -p
RM := rm -rf

# paths for library source/objects
LIB_DIR := src
LIB_SUBDIR := ctl ctl/list debug graph graph/opts hid ui utils

# main variables
LIB_DEPENDS:= -lfreetype -lpng -lz
LIB_INCLUDE:= -I./include
LIB_CFLAGS := -I$(LIB_DIR)
LIB_CFLAGS += -D_LARGEFILE64_SOURCE
LIB_CFLAGS += -Wl,--no-as-needed
# these are separate from LIB_DEPENDS to allow external dependencies in correct order when linking
LIB_STDLIBS = -lpthread -lm -lstdc++
# this to ignore invalid cflags (they depend on compiler)
LIB_CFLAGS+= -Wno-unused-command-line-argument -Wno-ignored-optimization-argument -Wno-unknown-warning-option

# general paths to be used
OUTDIR := out
ifeq ($(LIBAROMA_DEBUG_ENABLE),yes)
OBJDIR := $(OUTDIR)/obj-debug
else
OBJDIR := $(OUTDIR)/obj
endif #LIBAROMA_DEBUG_ENABLE yes

# target files
ifeq ($(OS),Windows_NT)
ifeq ($(LIBAROMA_DEBUG_ENABLE),yes)
LIB_SHARED := libaroma-debug.dll
else
LIB_SHARED := libaroma.dll
endif #LIBAROMA_DEBUG_ENABLE yes
else
ifeq ($(LIBAROMA_DEBUG_ENABLE),yes)
LIB_SHARED := libaroma-debug.so
else
LIB_SHARED := libaroma.so
endif #LIBAROMA_DEBUG_ENABLE yes
endif #OS Windows_NT
ifeq ($(LIBAROMA_DEBUG_ENABLE),yes)
LIB_STATIC := libaroma-debug.a
else
LIB_STATIC := libaroma.a
endif #LIBAROMA_DEBUG_ENABLE yes
ifeq ($(LIBAROMA_DEBUG_ENABLE),yes)
TEST_SHARED:= test_debug
TEST_STATIC:= test_debug-static
else
TEST_SHARED:= test
TEST_STATIC:= test-static
endif #LIBAROMA_DEBUG_ENABLE yes

# source & headers list
LIB_SRC := $(wildcard $(LIB_DIR)/*.c $(foreach fd, $(LIB_SUBDIR), $(LIB_DIR)/$(fd)/*.c))
LIB_HDR := $(wildcard $(LIB_DIR)/*.h $(foreach fd, $(LIB_SUBDIR), $(LIB_DIR)/$(fd)/*.c))

###
# CONFIGURATION PARSING SECTION
# 
# allow to override some build commands
ifneq ($(LIBAROMA_AR),)
AR := $(LIBAROMA_AR)
endif #LIBAROMA_AR
ifneq ($(LIBAROMA_CC),)
CC := $(LIBAROMA_CC)
endif #LIBAROMA_CC
ifneq ($(LIBAROMA_RANLIB),)
RANLIB := $(LIBAROMA_RANLIB)
endif #LIBAROMA_MKDIR
ifneq ($(LIBAROMA_MKDIR),)
MKDIR := $(LIBAROMA_MKDIR)
endif #LIBAROMA_MKDIR
ifneq ($(LIBAROMA_RM),)
RM := $(LIBAROMA_RM)
endif #LIBAROMA_RM
# check for platform requirements
ifeq ($(LIBAROMA_PLATFORM),linux)
# needed for shmem support
LIB_DEPENDS += -lrt
endif #LIBAROMA_PLATFORM linux
ifeq ($(LIBAROMA_PLATFORM),sdl)
LIB_CFLAGS += -DLIBAROMA_FB_INITHELPER
LIB_DEPENDS += -lSDL
else #LIBAROMA_PLATFORM sdl
ifeq ($(LIBAROMA_PLATFORM),sdl2)
LIB_CFLAGS += -DLIBAROMA_FB_INITHELPER -DLIBAROMA_PLATFORM_SDL2
LIB_DEPENDS += -lSDL2
# sdl2 platform uses same source folder as sdl
LIBAROMA_PLATFORM := sdl
endif #LIBAROMA_PLATFORM sdl2
endif #LIBAROMA_PLATFORM sdl
# include platform directory to sources
LIB_CFLAGS += -I$(LIB_DIR)/plat/$(LIBAROMA_PLATFORM)
LIB_SRC += $(wildcard $(LIB_DIR)/plat/$(LIBAROMA_PLATFORM)/*.c)
LIB_HDR += $(wildcard $(LIB_DIR)/plat/$(LIBAROMA_PLATFORM)/*.h)
ifeq ($(LIBAROMA_FEATURE_CPU),neon)
LIB_CFLAGS += -mfloat-abi=hard -mfpu=neon -D__ARM_NEON
else #LIBAROMA_FEATURE_CPU neon
ifeq ($(LIBAROMA_FEATURE_CPU),ssse3)
LIB_CFLAGS += -mssse3 -D__i386
endif #LIBAROMA_FEATURE_CPU ssse3
endif #LIBAROMA_FEATURE_CPU neon
# if debug enabled, check for debug level
ifeq ($(LIBAROMA_DEBUG_ENABLE),yes)
# if level not set, default to 5
ifeq ($(LIBAROMA_DEBUG_PRINTLEVEL),)
LIBAROMA_DEBUG_PRINTLEVEL := 5
endif #LIBAROMA_DEBUG_PRINTLEVEL
# if showfile disabled, set to 0
ifeq ($(LIBAROMA_DEBUG_SHOWFILE),no)
LIBAROMA_DEBUG_FILELINE := 0
else
# if enabled, show just filename (don't show line)
ifeq ($(LIBAROMA_DEBUG_SHOWFILE),yes)
LIBAROMA_DEBUG_FILELINE := 1
else #LIBAROMA_DEBUG_SHOWFILE yes
# otherwise, default to 2 (show filename & line)
LIBAROMA_DEBUG_FILELINE := 2
endif #LIBAROMA_DEBUG_SHOWFILE yes
endif #LIBAROMA_DEBUG_SHOWFILE withline
LIB_CFLAGS += -g -ggdb -DLIBAROMA_CONFIG_DEBUG_TRACE=3 -D_GLIBCXX_DEBUG_PEDANTIC -D_GLIBCXX_DEBUG
else #LIBAROMA_DEBUG_ENABLE yes
LIB_CFLAGS += -s -O3 -ftree-vectorize
endif #LIBAROMA_DEBUG_ENABLE yes
# defaults to minimal debug (only error/info/string, no file/line)
LIBAROMA_DEBUG_PRINTLEVEL ?= 3
LIBAROMA_DEBUG_FILELINE ?= 0
LIB_CFLAGS += -DLIBAROMA_CONFIG_DEBUG=$(LIBAROMA_DEBUG_PRINTLEVEL)
LIB_CFLAGS += -DLIBAROMA_CONFIG_DEBUG_FILE=$(LIBAROMA_DEBUG_FILELINE)
# if memtrack enabled, add it's source to list
ifeq ($(LIBAROMA_DEBUG_MEMTRACKER),yes)
LIB_SRC += $(LIB_DIR)/debug/memtrack.c
LIB_CFLAGS += -DLIBAROMA_CONFIG_DEBUG_MEMORY=1
endif
ifeq ($(LIBAROMA_FEATURE_JPEG),yes)
LIB_DEPENDS += -ljpeg
else
LIB_CFLAGS += -DLIBAROMA_CONFIG_NOJPEG
endif #LIBAROMA_FEATURE_JPEG yes
ifneq ($(LIBAROMA_FEATURE_HARFBUZZ),yes)
LIB_CFLAGS += -DLIBAROMA_CONFIG_TEXT_NOHARFBUZZ
endif #LIBAROMA_FEATURE_HARFBUZZ
ifeq ($(LIBAROMA_FEATURE_SVG),yes)
LIB_CFLAGS += -DLIBAROMA_CONFIG_NOSVG
endif #LIBAROMA_FEATURE_SVG
ifneq ($(LIBAROMA_FEATURE_OPENMP),no)
LIB_CFLAGS += -DLIBAROMA_CONFIG_OPENMP -fopenmp
endif #LIBAROMA_FEATURE_OPENMP no
ifeq ($(LIBAROMA_PLATFORM),linux)
ifeq ($(LIBAROMA_FEATURE_DRM),no)
LIB_CFLAGS += -DLIBAROMA_CONFIG_NODRM
else #LIBAROMA_FEATURE_DRM no
LIB_CFLAGS += -I$(LIB_DIR)/drm
LIB_SRC += $(wildcard $(LIB_DIR)/drm/*.c)
LIB_HDR += $(wildcard $(LIB_DIR)/drm/*.h)
endif #LIBAROMA_FEATURE_DRM no
endif #LIBAROMA_PLATFORM linux
ifeq ($(LIBAROMA_FEATURE_MINZIP),no)
LIB_CFLAGS += -DLIBAROMA_CONFIG_NOMINZIP
else #LIBAROMA_FEATURE_MINZIP no
LIB_CFLAGS += -I$(LIB_DIR)/minzip
LIB_SRC += $(wildcard $(LIB_DIR)/minzip/*.c)
LIB_HDR += $(wildcard $(LIB_DIR)/minzip/*.h)
endif #LIBAROMA_FEATURE_MINZIP no
ifneq ($(LIBAROMA_TTY_KDSETMODE),)
LIB_CFLAGS += -DLIBAROMA_TTY_KDSETMODE=$(LIBAROMA_TTY_KDSETMODE)
endif #LIBAROMA_TTY_KDSETMODE
ifeq ($(LIBAROMA_COMPILE_WARN),all)
LIB_CFLAGS += -Wall -Wextra
else #LIBAROMA_COMPILE_WARN all
ifeq ($(LIBAROMA_COMPILE_WARN),none)
LIB_CFLAGS += -w
endif #LIBAROMA_COMPILE_WARN none
endif #LIBAROMA_COMPILE_WARN all
ifeq ($(LIBAROMA_COMPILE_WARNERROR),yes)
LIB_CFLAGS += -Werror
endif #LIBAROMA_COMPILE_WARNERROR yes
ifeq ($(LIBAROMA_COMPILE_SHOWINFO),yes)
LIB_CFLAGS += -DLIBAROMA_CONFIG_COMPILER_MESSAGE=1
else #LIBAROMA_COMPILE_SHOWINFO yes
LIB_CFLAGS += -DLIBAROMA_CONFIG_COMPILER_MESSAGE=0
endif #LIBAROMA_COMPILE_SHOWINFO yes
ifeq ($(LIBAROMA_COMPILE_NOCONSOLE),yes)
LIB_CFLAGS += -mwindows
endif #LIBAROMA_COMPILE_NOCONSOLE yes
ifneq ($(LIBAROMA_COMPILE_PIC),no)
LIB_CFLAGS += -fPIC -DPIC
endif #LIBAROMA_COMPILE_PIC yes
# use additional include paths
ifneq ($(LIBAROMA_COMPILE_INCLUDES),)
LIBAROMA_COMPILE_INCLUDES := $(addprefix -I,$(LIBAROMA_COMPILE_INCLUDES))
LIB_CFLAGS += $(LIBAROMA_COMPILE_INCLUDES)
endif
# don't override prefix if provided
ifneq ($(PREFIX),)
# read it from config if there
ifneq ($(LIBAROMA_INSTALL_PREFIX),)
PREFIX := $(LIBAROMA_INSTALL_PREFIX)
endif #LIBAROMA_INSTALL_PREFIX
else #LIBAROMA_INSTALL_PREFIX
PREFIX := /usr
endif #LIBAROMA_INSTALL_PREFIX

###
# MAKE DEFAULTS SECTION
#
# ignore any file with the same name as these recipes
.PHONY: default all clean install install-static uninstall

# build shared library by default (it's the first recipe)
default: $(LIB_SHARED)

# tests depend on libs so they will be built if needed
all: $(TEST_SHARED) $(TEST_STATIC)

###
# TEST/EXAMPLE BUILDING SECTION
# 

# cflags for linking with lib
TEST_CFLAGS := -L$(OUTDIR) -laroma

# paths for test program
TEST_SRC := $(wildcard examples/fulltest/*.c)
TEST_HDR := $(wildcard examples/fulltest/*.h)

# build test program
$(TEST_SHARED): $(TEST_SRC) $(TEST_HDR) $(LIB_SHARED)
	$(info Building $@)
	@$(CC) -o $(OUTDIR)/$@ $(TEST_SRC) $(LIB_INCLUDE) $(LIB_CFLAGS) $(TEST_CFLAGS)

# build static test program
$(TEST_STATIC): $(TEST_SRC) $(TEST_HDR) $(LIB_STATIC)
	$(info Building $@)
	@$(CC) -static -o $(OUTDIR)/$@ $(LIB_INCLUDE) $(TEST_SRC) $(LIB_CFLAGS) $(TEST_CFLAGS) $(LIB_DEPENDS) $(LIBAROMA_LIBDEPENDS) $(LIB_STDLIBS)

###
# MAIN LIBRARY BUILDING SECTION
# 

# target objects list
# this goes after config parsing to include any config-dependant sources
LIB_OBJ := $(addprefix $(OBJDIR)/, $(LIB_SRC:.c=.o))

# recipe aliases
shared: $(LIB_SHARED)
static: $(LIB_STATIC)

# build shared library from objects list
$(LIB_SHARED): $(LIB_OBJ) $(LIB_HDR) config.txt
	$(info Building $@)
	@$(CC) -shared -o $(OUTDIR)/$(LIB_SHARED) $(LIB_INCLUDE) $(LIB_CFLAGS) $(LIB_OBJ) $(LIB_DEPENDS) $(LIBAROMA_LIBDEPENDS) $(LIB_STDLIBS)

# build static library from objects list
$(LIB_STATIC): $(LIB_OBJ) $(LIB_HDR) config.txt
	$(info Building $@)
	@$(AR) -r -c -s $(OUTDIR)/$@ $(LIB_OBJ)
	@$(RANLIB) $(OUTDIR)/$@

# build required object from c file
$(OBJDIR)/%.o: %.c $(LIB_HDR)
	@$(MKDIR) $(@D)
	@$(CC) -c -o $@ $< $(LIB_CFLAGS) $(LIB_INCLUDE)

###
# MAKE MUST-HAVE RECIPES
#

clean:
	$(info Deleting $(OUTDIR)/*)
	@$(RM) $(OBJDIR)/*
	@$(RM) $(OUTDIR)/*

install: $(LIB_SHARED)
	$(info Installing shared library to $(DESTDIR)$(PREFIX))
	@install -Dm 644 include/aroma.h -t $(DESTDIR)$(PREFIX)/include
	@install -Dm 644 include/aroma/* -t $(DESTDIR)$(PREFIX)/include/aroma
	@install -Dm 644 $(OUTDIR)/$(LIB_SHARED) -t $(DESTDIR)$(PREFIX)/lib

install-static: $(LIB_STATIC)
	$(info Installing static library to $(DESTDIR)$(PREFIX))
	@install -Dm 644 include/aroma.h -t $(DESTDIR)$(PREFIX)/include
	@install -Dm 644 include/aroma/* -t $(DESTDIR)$(PREFIX)/include/aroma
	@install -Dm 644 $(OUTDIR)/$(LIB_STATIC) -t $(DESTDIR)$(PREFIX)/lib

uninstall:
	$(info Deleting installed library)
	$(info $(DESTDIR)$(PREFIX)/include/aroma.h)
	$(info $(DESTDIR)$(PREFIX)/include/aroma/)
	$(info $(DESTDIR)$(PREFIX)/lib/$(LIB_SHARED))
	$(info $(DESTDIR)$(PREFIX)/lib/$(LIB_STATIC))
	@$(RM) $(DESTDIR)$(PREFIX)/include/aroma.h
	@$(RM) $(DESTDIR)$(PREFIX)/include/aroma/
	@$(RM) $(DESTDIR)$(PREFIX)/lib/$(LIB_SHARED)
	@$(RM) $(DESTDIR)$(PREFIX)/lib/$(LIB_STATIC)
