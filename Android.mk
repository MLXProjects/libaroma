####################################################################[libaroma]#
#  Copyright (C) 2011-2023 Ahmad Amarullah (http://amarullz.com/)
# 
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
# 
#       http://www.apache.org/licenses/LICENSE-2.0
# 
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
# ______________________________________________________________________________
# 
#  Filename    : Android.mk
#  Description : Android make file
# 
#  + This is part of libaroma, an embedded ui toolkit.
#  + 16/01/23 - Author(s): Frantisek Kysela, Michael Jauregui
#
####################################################################[libaroma]#

# almost mandatory for any Android makefile
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

###
# default settings (TODO: should be overridable by device tree)
#
LIBAROMA_PLATFORM := linux
LIBAROMA_FEATURE_CPU := neon
LIBAROMA_FEATURE_OPENMP := true
LIBAROMA_FEATURE_MINZIP := true
LIBAROMA_FEATURE_HARFBUZZ := false
LIBAROMA_FEATURE_JPEG := true
LIBAROMA_FEATURE_SVG := true
LIBAROMA_DEBUG_PRINTLEVEL := 5
LIBAROMA_DEBUG_SHOWFILE := withline
LIBAROMA_COMPILE_BUILDINFO := true
LIBAROMA_COMPILE_WARN := all
LIBAROMA_COMPILE_WARNERROR := true

# default lib include paths for AOSP build system, only used if variables not set
LIBAROMA_FEATURE_FREETYPE_PATH ?= external/freetype/include
LIBAROMA_FEATURE_PNG_PATH ?= external/libpng
LIBAROMA_FEATURE_HARFBUZZ_PATH ?= external/harfbuzz_ng/src
LIBAROMA_FEATURE_JPEG_PATH ?= external/jpeg
LIBAROMA_FEATURE_ZLIB_PATH ?= external/zlib

###
# shared library module definitions
#
# main module
include $(CLEAR_VARS)
LOCAL_MODULE := libaroma
LOCAL_MODULE_TAGS := eng

# default cflags
LOCAL_CFLAGS := -fPIC -DPIC

# main source files, includes & dependencies
LOCAL_SRC_FILES := \
	src/aroma.c \
	src/ctl/bar.c \
	src/ctl/btn.c \
	src/ctl/clock.c \
	src/ctl/frag.c \
	src/ctl/img.c \
	src/ctl/label.c \
	src/ctl/list.c \
	src/ctl/pager.c \
	src/ctl/prog.c \
	src/ctl/scroll.c \
	src/ctl/slider.c \
	src/ctl/tabs.c \
	src/ctl/list/caption.c \
	src/ctl/list/check.c \
	src/ctl/list/div.c \
	src/ctl/list/img.c \
	src/ctl/list/menu.c \
	src/ctl/list/option.c \
	src/ctl/list/text.c \
	src/graph/art.c \
	src/graph/cv.c \
	src/graph/draw.c \
	src/graph/draw_blur.c \
	src/graph/draw_filters.c \
	src/graph/draw_grad.c \
	src/graph/draw_path.c \
	src/graph/draw_scale.c \
	src/graph/engine_alignblt.c \
	src/graph/engine_alpha.c \
	src/graph/engine_blt.c \
	src/graph/engine_color.c \
	src/graph/engine_dither.c \
	src/graph/fb.c \
	src/graph/img.c \
	src/graph/jpeg.c \
	src/graph/png.c \
	src/graph/svg.c \
	src/graph/text_chunk.c \
	src/graph/text_common.c \
	src/graph/text_draw.c \
	src/graph/text_ft.c \
	src/graph/text_hb.c \
	src/graph/text_line.c \
	src/graph/text_public.c \
	src/graph/text_shaper.c \
	src/graph/text_tags.c \
	src/graph/text_ucdn.c \
	src/hid/hid.c \
	src/hid/msg.c \
	src/ui/colormgr.c \
	src/ui/ctl.c \
	src/ui/dlg.c \
	src/ui/win.c \
	src/ui/winlayer.c \
	src/ui/winmgr.c \
	src/utils/array.c \
	src/utils/json.c \
	src/utils/motions.c \
	src/utils/stream.c \
	src/utils/string.c \
	src/utils/sys.c \
	src/utils/time.c \
	src/utils/utf8.c \
	src/utils/xml.c \
	src/utils/zip.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/src \
	$(LIBAROMA_FEATURE_ZLIB_PATH) \
	$(LIBAROMA_FEATURE_PNG_PATH) \
	$(LIBAROMA_FEATURE_FREETYPE_PATH)

LOCAL_SHARED_LIBRARIES := libz libpng libft2

###
# configuration parsing
#
# platform
ifeq ($(LIBAROMA_PLATFORM),sdl2)
LOCAL_CFLAGS += -DLIBAROMA_PLATFORM_SDL2
# SDL & SDL2 share platform source
LIBAROMA_PLATFORM := sdl
endif #PLATFORM sdl2
# setup platform includes & source files
ifeq ($(LIBAROMA_PLATFORM),linux)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src/plat/linux
LOCAL_SRC_FILES += \
	src/plat/linux/aroma_plat.c \
	src/plat/linux/fb_16bit.c \
	src/plat/linux/fb_32bit.c \
	src/plat/linux/fb.c \
	src/plat/linux/fb_qcom.c \
	src/plat/linux/input.c \
	src/plat/linux/input_kb.c \
	src/plat/linux/input_pointer.c \
	src/plat/linux/input_touch.c
else
ifeq ($(LIBAROMA_PLATFORM),sdl)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src/plat/sdl
LOCAL_SRC_FILES += \
	src/plat/sdl/aroma_plat.c \
	src/plat/sdl/fb.c \
	src/plat/sdl/input.c
else #defaults to whether platform you've set
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src/plat/$(LIBAROMA_PLATFORM)
LOCAL_SRC_FILES += \
	src/plat/$(LIBAROMA_PLATFORM)/aroma_plat.c \
	src/plat/$(LIBAROMA_PLATFORM)/fb.c \
	src/plat/$(LIBAROMA_PLATFORM)/input.c
endif #PLATFORM sdl
endif #PLATFORM linux
# features
# check for cpu optimizations
ifeq ($(LIBAROMA_FEATURE_CPU),neon)
LOCAL_CFLAGS += -D__ARM_NEON
else #FEATURE_CPU neon
ifeq ($(LIBAROMA_FEATURE_CPU),ssse3)
LOCAL_CFLAGS += -mssse3 -D__i386
endif #FEATURE_CPU ssse3
endif #FEATURE_CPU neon
# check for jpeg enabled
ifeq ($(LIBAROMA_FEATURE_JPEG),true)
LOCAL_C_INCLUDES += $(LIBAROMA_FEATURE_JPEG_PATH)
LOCAL_SHARED_LIBRARIES += libjpeg
else
LOCAL_CFLAGS += -DLIBAROMA_CONFIG_NOJPEG
endif #FEATURE_JPEG true
# check for harfbuzz enabled
ifeq ($(LIBAROMA_FEATURE_HARFBUZZ),true)
LOCAL_C_INCLUDES += $(LIBAROMA_FEATURE_HARFBUZZ_PATH)
LOCAL_SHARED_LIBRARIES += libharfbuzz_ng
else
LOCAL_CFLAGS += -DLIBAROMA_CONFIG_TEXT_NOHARFBUZZ
endif #FEATURE_HARFBUZZ true
# check for minzip disabled
ifeq ($(LIBAROMA_FEATURE_MINZIP),false)
LIBAROMA_CFLAGS += -DLIBAROMA_CONFIG_NOMINZIP
else
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src/minzip
LOCAL_SRC_FILES += \
	src/minzip/DirUtil.c \
	src/minzip/Hash.c \
	src/minzip/Inlines.c \
	src/minzip/SysUtil.c \
	src/minzip/Zip.c
endif
# check for svg disabled
ifeq ($(LIBAROMA_FEATURE_SVG),false)
LIBAROMA_CFLAGS += -DLIBAROMA_CONFIG_NOSVG
endif
# check if debug enabled
ifeq ($(LIBAROMA_DEBUG_ENABLE),yes)
# if print level not set, default to 5
ifeq ($(LIBAROMA_DEBUG_PRINTLEVEL),)
LIBAROMA_DEBUG_PRINTLEVEL := 5
endif
# if showfile disabled, set to 0
ifeq ($(LIBAROMA_DEBUG_SHOWFILE),no)
LIBAROMA_DEBUG_FILELINE := 0
else
# if enabled, show just filename (don't show line)
ifeq ($(LIBAROMA_DEBUG_SHOWFILE),true)
LIBAROMA_DEBUG_FILELINE := 1
else #DEBUG_SHOWFILE true
# otherwise, default to 2 (show filename & line)
LIBAROMA_DEBUG_FILELINE := 2
endif #DEBUG_SHOWFILE true
endif #DEBUG_SHOWFILE withline
# set cflags to enable debug
LOCAL_CFLAGS += -g -ggdb -DLIBAROMA_CONFIG_DEBUG_TRACE=3
else #DEBUG_ENABLE true
LOCAL_CFLAGS += -s -O3 -ftree-vectorize
endif #DEBUG_ENABLE true
# defaults to minimal debug (only error/info/string, no file/line)
LIBAROMA_DEBUG_PRINTLEVEL ?= 3
LIBAROMA_DEBUG_FILELINE ?= 0
LOCAL_CFLAGS += -DLIBAROMA_CONFIG_DEBUG=$(LIBAROMA_DEBUG_PRINTLEVEL)
LOCAL_CFLAGS += -DLIBAROMA_CONFIG_DEBUG_FILE=$(LIBAROMA_DEBUG_FILELINE)
# if memtrack enabled, add it's source to list
ifeq ($(LIBAROMA_DEBUG_MEMTRACKER),true)
LOCAL_SRC_FILES += src/debug/memtrack.c
LOCAL_CFLAGS += -DLIBAROMA_CONFIG_DEBUG_MEMORY=1
endif
# misc (kdsetmode, warnings...)
# check for kdsetmode enabled
ifneq ($(LIBAROMA_TTY_KDSETMODE),)
LOCAL_CFLAGS += -DLIBAROMA_TTY_KDSETMODE=$(LIBAROMA_TTY_KDSETMODE)
endif
# check for specific warning settings
ifeq ($(LIBAROMA_COMPILE_WARN),all)
LOCAL_CFLAGS += -Wall -Wextra
else
ifeq ($(LIBAROMA_COMPILE_WARN),none)
LOCAL_CFLAGS += -w
endif #COMPILE_WARN none
endif #COMPILE_WARN all
# check if treating warning as errors
ifeq ($(LIBAROMA_COMPILE_WARNERROR),true)
LOCAL_CFLAGS += -Werror
endif
# check if must show build info
ifeq ($(LIBAROMA_COMPILE_BUILDINFO),true)
LOCAL_CFLAGS += -DLIBAROMA_CONFIG_COMPILER_MESSAGE=1
endif

###
# build the shared library after setting up
#
include $(BUILD_SHARED_LIBRARY)
