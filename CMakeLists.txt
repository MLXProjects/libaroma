###
# main CMake project info
#
cmake_minimum_required(VERSION 3.10)
project(libaroma LANGUAGES C ASM)

# find required dependencies
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenMP)
find_package(JPEG)

###
# build settings
#
option(BUILD_SHARED_LIBS "Build shared library" YES)
option(BUILD_STATIC_LIBS "Build static library" YES)
set(LIBAROMA_CONFIG_HICOLOR_BIT 0 CACHE STRING "High color bit")
option(LIBAROMA_CONFIG_SIMD "Use SIMD optimizations where applicable" YES)
option(LIBAROMA_CONFIG_OPENMP "Use OpenMP" YES)
option(LIBAROMA_CONFIG_NOZIP "Disable Zip support" YES)
option(LIBAROMA_CONFIG_NOJPEG "Disable JPEG" YES)
option(LIBAROMA_CONFIG_NOPTHREAD "Linux platform: Disable pthread" NO)
option(LIBAROMA_CONFIG_TEXT_NOHARFBUZZ "Disable Harfbuzz text shaping" YES)
option(LIBAROMA_CONFIG_NOFONT_SUBPIXEL "Disable cleartype-like text antialiasing" NO)
option(LIBAROMA_CONFIG_TEXT_GLOBAL_LOCK "Enable text global lock" NO)
set(LIBAROMA_CONFIG_DEBUG 3 CACHE STRING "Debug: level (0-7)")
set(LIBAROMA_CONFIG_DEBUG_FILE 0 CACHE STRING "Debug: show file (0-2)")
option(LIBAROMA_CONFIG_DEBUG_TRACE "Debug: memory backtracing" NO)
option(LIBAROMA_CONFIG_DEBUG_MEMORY "Debug: show memory allocations" NO)
option(LIBAROMA_CONFIG_COMPILER_MESSAGE "Show build settings" YES)
option(LIBAROMA_CONFIG_SHMEMFB "Enable shared memory framebuffer" YES)
set(LIBAROMA_CONFIG_SHMEM_PREFIX "/aromashm-" CACHE STRING "Shared memory framebuffer prefix")
set(SUPPORTED_PLATFORMS linux qnx sdl)
set(LIBAROMA_PLATFORM "linux" CACHE STRING "Target platform")

###
# architecture detection
#
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" TARGET_ARCH)
if (TARGET_ARCH MATCHES "x86_64|amd64")
	set(ARCH_MEMSET src/aroma/arch/x86_64/android_memset16.S src/aroma/arch/x86_64/android_memset32.S)
elseif (TARGET_ARCH MATCHES "x86|i[3-6]86")
	set(ARCH_MEMSET src/aroma/arch/x86/android_memset16.S src/aroma/arch/x86/android_memset32.S)
elseif (TARGET_ARCH MATCHES "armv7l|arm")
	set(ARCH_MEMSET src/aroma/arch/arm/memset32.S)
elseif (TARGET_ARCH MATCHES "aarch64|arm64")
	set(ARCH_MEMSET src/aroma/arch/arm64/android_memset.S)
else()
	# fallback to non-assembler memset
	set(ARCH_MEMSET src/aroma/arch/mips/android_memset.c)
endif()

###
# setting dependency checks
if (NOT OpenMP_C_FOUND)
	set(LIBAROMA_CONFIG_OPENMP NO)
endif()
if (NOT JPEG_FOUND)
	set(LIBAROMA_CONFIG_NOJPEG YES)
endif()

###
# platform selection
#
set_property(CACHE LIBAROMA_PLATFORM PROPERTY STRINGS ${SUPPORTED_PLATFORMS})
if (NOT LIBAROMA_PLATFORM IN_LIST SUPPORTED_PLATFORMS)
	message(FATAL_ERROR "Unsupported platform: ${LIBAROMA_PLATFORM}. Choose from: ${SUPPORTED_PLATFORMS}")
endif()
if (LIBAROMA_PLATFORM STREQUAL sdl)
	find_package(SDL REQUIRED)
endif()
set(PLATFORM_SOURCES
	src/contrib/platform/${LIBAROMA_PLATFORM}/platform.c
	src/contrib/platform/${LIBAROMA_PLATFORM}/fb_driver.c
	src/contrib/platform/${LIBAROMA_PLATFORM}/hid_driver.c
)

###
# source files
#
set(LIB_SOURCES
	src/aroma/debug/compiler_message.c
	${ARCH_MEMSET}
	src/aroma/aroma.c
	src/aroma/version.c
	src/aroma/controls/ctl_bar.c
	src/aroma/controls/ctl_button.c
	src/aroma/controls/ctl_fragment.c
	src/aroma/controls/ctl_image.c
	src/aroma/controls/ctl_label.c
	src/aroma/controls/ctl_list.c
	src/aroma/controls/ctl_pager.c
	src/aroma/controls/ctl_progress.c
	src/aroma/controls/ctl_scroll.c
	src/aroma/controls/ctl_tabs.c
	src/aroma/controls/listitem/listitem_caption.c
	src/aroma/controls/listitem/listitem_check.c
	src/aroma/controls/listitem/listitem_divider.c
	src/aroma/controls/listitem/listitem_image.c
	src/aroma/controls/listitem/listitem_menu.c
	src/aroma/controls/listitem/listitem_option.c
	src/aroma/debug/memory_tracking.c
	src/aroma/graph/artworker.c
	src/aroma/graph/canvas.c
	src/aroma/graph/fb.c
	src/aroma/graph/image.c
	src/aroma/graph/jpeg.c
	src/aroma/graph/png.c
	src/aroma/graph/svg.c
	src/aroma/graph/text.c
	src/aroma/graph/draw/blur.c
	src/aroma/graph/draw/commondraw.c
	src/aroma/graph/draw/filters.c
	src/aroma/graph/draw/gradient.c
	src/aroma/graph/draw/path.c
	src/aroma/graph/draw/scale.c
	src/aroma/graph/engine/alignblt.c
	src/aroma/graph/engine/alpha.c
	src/aroma/graph/engine/blt.c
	src/aroma/graph/engine/color.c
	src/aroma/graph/engine/dither.c
	src/aroma/graph/text/ucdn.c
	src/aroma/hid/hid.c
	src/aroma/hid/messages.c
	src/aroma/ui/color_manager.c
	src/aroma/ui/control.c
	src/aroma/ui/dialog.c
	src/aroma/ui/window.c
	src/aroma/ui/window_layer.c
	src/aroma/ui/window_manager.c
	src/aroma/utils/array.c
	src/aroma/utils/json.c
	src/aroma/utils/minutf8.c
	src/aroma/utils/motions.c
	src/aroma/utils/runtime.c
	src/aroma/utils/stream.c
	src/aroma/utils/strings.c
	src/aroma/utils/system.c
	src/aroma/utils/time.c
	src/aroma/utils/zip.c
	${PLATFORM_SOURCES}
)

###
# set build targets
#
set(LIB_TARGETS "")
if (BUILD_SHARED_LIBS)
	add_library(aroma_shared SHARED ${LIB_SOURCES})
	list(APPEND LIB_TARGETS aroma_shared)
endif()
if (BUILD_STATIC_LIBS)
	add_library(aroma_static STATIC ${LIB_SOURCES})
	list(APPEND LIB_TARGETS aroma_static)
endif()
foreach (target ${LIB_TARGETS})
	# public include directory
	target_include_directories(${target} PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	)
	# internal headers
	target_include_directories(${target} PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${CMAKE_CURRENT_SOURCE_DIR}/src/aroma
		${CMAKE_CURRENT_SOURCE_DIR}/src/contrib/platform/${LIBAROMA_PLATFORM}/include
	)
	# build options
	target_compile_definitions(${target} PRIVATE
		LIBAROMA_CONFIG_DEBUG=${LIBAROMA_CONFIG_DEBUG}
		LIBAROMA_CONFIG_DEBUG_FILE=${LIBAROMA_CONFIG_DEBUG_FILE})
	if (LIBAROMA_CONFIG_DEBUG_MEMORY)
		target_compile_definitions(${target} PRIVATE LIBAROMA_CONFIG_DEBUG_MEMORY=1)
	endif()
	if (LIBAROMA_CONFIG_DEBUG_TRACE)
		target_compile_definitions(${target} PRIVATE LIBAROMA_CONFIG_DEBUG_TRACE=1)
	endif()
	if (LIBAROMA_CONFIG_SHMEMFB)
		target_compile_definitions(${target} PRIVATE
			LIBAROMA_CONFIG_SHMEMFB=1
			LIBAROMA_CONFIG_SHMEM_PREFIX="{LIBAROMA_CONFIG_SHMEM_PREFIX}")
	endif()
	if (LIBAROMA_CONFIG_NOPTHREAD)
		target_compile_definitions(${target} PRIVATE LIBAROMA_CONFIG_NOPTHREAD=1)
	endif()
	if (LIBAROMA_CONFIG_NOZIP)
		target_compile_definitions(${target} PRIVATE LIBAROMA_CONFIG_NOZIP=1)
	endif()
	if (LIBAROMA_CONFIG_TEXT_GLOBAL_LOCK)
		target_compile_definitions(${target} PRIVATE LIBAROMA_CONFIG_TEXT_GLOBAL_LOCK=1)
	endif()
	if (LIBAROMA_CONFIG_TEXT_NOHARFBUZZ)
		target_compile_definitions(${target} PRIVATE LIBAROMA_CONFIG_TEXT_NOHARFBUZZ=1)
	endif()
	if (LIBAROMA_CONFIG_NOFONT_SUBPIXEL)
		target_compile_definitions(${target} PRIVATE LIBAROMA_CONFIG_NOFONT_SUBPIXEL=1)
	endif()
	if (LIBAROMA_CONFIG_NOJPEG)
		target_compile_definitions(${target} PRIVATE LIBAROMA_CONFIG_NOJPEG=1)
	endif()
	if (LIBAROMA_CONFIG_SIMD)
		target_compile_definitions(${target} PRIVATE LIBAROMA_CONFIG_SIMD=1)
	endif()
	if (LIBAROMA_CONFIG_HICOLOR_BIT)
		target_compile_definitions(${target} PRIVATE LIBAROMA_CONFIG_HICOLOR_BIT=1)
	endif()
	if (LIBAROMA_CONFIG_COMPILER_MESSAGE)
		target_compile_definitions(${target} PRIVATE LIBAROMA_CONFIG_COMPILER_MESSAGE=1)
	endif()
   	# add configs with libraries
	if (LIBAROMA_CONFIG_OPENMP)
		target_compile_definitions(${target} PRIVATE
			LIBAROMA_CONFIG_OPENMP=1
		)
		target_link_libraries(${target} PRIVATE OpenMP::OpenMP_C)
	endif()
	# link basic dependencies
	target_link_libraries(${target} PRIVATE 
		ZLIB::ZLIB
		PNG::PNG
		Freetype::Freetype
		m
		rt
	)
	if (LIBAROMA_PLATFORM STREQUAL sdl)
		target_link_libraries(${target} PRIVATE SDL::SDL)
	endif()
	if (NOT LIBAROMA_CONFIG_NOJPEG)
		target_link_libraries(${target} PRIVATE JPEG::JPEG)
	endif()
	# set output names
	set_target_properties(${target} PROPERTIES OUTPUT_NAME "aroma")
endforeach()

###
# install rules
#
install(TARGETS ${LIB_TARGETS}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
#install(DIRECTORY include/
#	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#)


###
# example program
#
foreach (target ${LIB_TARGETS})
	add_executable(test-${target} examples/libaroma_test.c)
	target_link_libraries(test-${target} PRIVATE ${target})
	target_include_directories(test-${target} PRIVATE 
		${CMAKE_CURRENT_SOURCE_DIR}/include
	)
endforeach()
