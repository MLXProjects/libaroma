@echo off
if not defined _LIBAROMA_ENVSETUP_ROOT (
	echo Please use envsetup at source root
	goto :eof
)

set LIBAROMA_BASE=%_LIBAROMA_ENVSETUP_ROOT%

for /f "tokens=1,2 delims==" %%a in (config-win.txt) do (
	rem get key and value from tokens
	set key=%%a
	set val=%%b
	rem remove spaces before and after the key name
	call :trim key
	rem ignore comment lines
	if not "!key:~0,1!" == "#" (
		rem if vaule is not empty, set the variable
		if not "!val!" == "" (
			call :trim val
			set !key!=!val!
		) else (
			rem otherwise, warn that the variable is empty
			echo variable !key! not set
		)
	)
)

if not defined LIBAROMA_DEBUG_ENABLED set LIBAROMA_DEBUG_ENABLED=0
if "%1" == "d" (
	set LIBAROMA_DEBUG_ENABLED=1
)

if not defined LIBAROMA_ARCH set LIBAROMA_ARCH=arm
if not defined LIBAROMA_ARCH_APPEND set "LIBAROMA_ARCH_APPEND= "
if not defined LIBAROMA_PLATFORM set LIBAROMA_PLATFORM=linux

if not defined LIBAROMA_TOOLCHAIN_PREFIX set "LIBAROMA_TOOLCHAIN_PREFIX= "
if not defined LIBAROMA_GCC set LIBAROMA_GCC=%LIBAROMA_TOOLCHAIN_PREFIX%gcc
if not defined LIBAROMA_GPP set LIBAROMA_GPP=%LIBAROMA_TOOLCHAIN_PREFIX%g++

rem surround path with quotes if it contains spaces, only for compilers
for %%a in ("%LIBAROMA_GCC:"=%") do (
	if not ^%LIBAROMA_GCC:~0,1% == ^" if not ^%LIBAROMA_TOOLCHAIN_PREFIX:~-1,1% == ^" (
		set LIBAROMA_GCC="%LIBAROMA_GCC%"
	)
)

for %%a in ("%LIBAROMA_GPP:"=%") do (
	if not ^%LIBAROMA_GPP:~0,1% == ^" if not ^%LIBAROMA_TOOLCHAIN_PREFIX:~-1,1% == ^" (
		set LIBAROMA_GPP="%LIBAROMA_GPP%"
	)
)

rem some default values
set "LIBAROMA_APP_STATIC= "
set "LIBAROMA_ADDITIONAL_LIBS= "
set "LIBAROMA_FREETYPE_CFLAGS= "
set "LIBAROMA_STRIP_OBJECT= "
if not defined LIBAROMA_BUILD_DRM set LIBAROMA_BUILD_DRM=0
if not defined LIBAROMA_BUILD_HARFBUZZ set LIBAROMA_BUILD_HARFBUZZ=1
if not defined LIBAROMA_BUILD_JPEG (
	if "%LIBAROMA_ARCH_APPEND%"=="neon" (
		set LIBAROMA_BUILD_JPEG=1
	) else set LIBAROMA_BUILD_JPEG=0
) 
if not defined LIBAROMA_BUILD_MINUI set LIBAROMA_BUILD_MINUI=0
if not defined LIBAROMA_BUILD_RUNTIME set LIBAROMA_BUILD_RUNTIME=0
if not defined LIBAROMA_CONFIG_DEBUG set LIBAROMA_CONFIG_DEBUG=3
if not defined LIBAROMA_CONFIG_DEBUG_FILE set LIBAROMA_CONFIG_DEBUG_FILE=0

set "LIBAROMA_CFLAGS= "

if not defined LIBAROMA_CONFIG_OPENMP set LIBAROMA_CONFIG_OPENMP=1

if %LIBAROMA_CONFIG_OPENMP% == 1 (
	set LIBAROMA_CFLAGS=%LIBAROMA_CFLAGS% -fopenmp -DLIBAROMA_CONFIG_OPENMP=1
)

if not defined LIBAROMA_COMPILER_WARNINGS set LIBAROMA_COMPILER_WARNINGS=0

if %LIBAROMA_COMPILER_WARNINGS% == 2 ( set LIBAROMA_CFLAGS=%LIBAROMA_CFLAGS% -Wextra
) else if %LIBAROMA_COMPILER_WARNINGS% == 1 ( set LIBAROMA_CFLAGS=%LIBAROMA_CFLAGS% -Wall
) else set LIBAROMA_CFLAGS=%LIBAROMA_CFLAGS% -w

if not defined LIBAROMA_COMPILER_ERROR_ON_WARN set LIBAROMA_COMPILER_ERROR_ON_WARN=0
if %LIBAROMA_COMPILER_ERROR_ON_WARN% == 1 set LIBAROMA_CFLAGS=%LIBAROMA_CFLAGS% -Werror

rem if building for android, revert platform to linux 
rem and enable android in cflags
if "%LIBAROMA_PLATFORM%" == "android" (
	set LIBAROMA_CFLAGS=%LIBAROMA_CFLAGS% -DANDROID=1 -D__ANDROID__
	set LIBAROMA_PLATFORM=linux
)

rem if debug is enabled, use minimal cflags with debug.
rem otherwise, use optimizations and strip output
if %LIBAROMA_DEBUG_ENABLED% == 1 (
	set LIBAROMA_DEBUG_SUFFIX=-debug
	set LIBAROMA_CFLAGS=%LIBAROMA_CFLAGS% -g -ggdb -DLIBAROMA_CONFIG_DEBUG_TRACE=3 -D_GLIBCXX_DEBUG_PEDANTIC -D_GLIBCXX_DEBUG
) else (
	set LIBAROMA_DEBUG_SUFFIX=-release
	set LIBAROMA_CFLAGS=%LIBAROMA_CFLAGS% -s -O3 -ftree-vectorize -fprefetch-loop-arrays -funsafe-math-optimizations
	set LIBAROMA_STRIP_OBJECT=-Wl,-s
)

rem if targetting sdl/sdl2, use init helper (SDL window creation and input loop are done in same thread)
rem This is needed because SDL doesn't seem to like handling requests from multiple threads :/
if "%LIBAROMA_PLATFORM%" == "sdl" set LIBAROMA_CFLAGS=%LIBAROMA_CFLAGS% -DLIBAROMA_INIT_HELPER
if "%LIBAROMA_PLATFORM%" == "sdl2" set LIBAROMA_CFLAGS=%LIBAROMA_CFLAGS% -DLIBAROMA_INIT_HELPER

rem enable architecture optimizations
if "%LIBAROMA_ARCH%" == "arm" (
	if "%LIBAROMA_ARCH_APPEND%" == "neon" (
		set LIBAROMA_CFLAGS=%LIBAROMA_CFLAGS% -mfloat-abi=hard -mfpu=neon -D__ARM_HAVE_NEON -mvectorize-with-neon-quad
	)
) else (
	if "%LIBAROMA_ARCH%" == "x86" set i386=1
	if "%LIBAROMA_ARCH%" == "x86_64" set i386=1
	if "!i386!" == "1" (
		set LIBAROMA_CFLAGS=%LIBAROMA_CFLAGS% -D__i386 
	)
	set "i386="
)

if not "%LIBAROMA_ARCH_APPEND%" == "neon" (
	set LIBAROMA_CFLAGS=%LIBAROMA_CFLAGS% -DLIBAROMA_CONFIG_NOJPEG=1
)

rem link with SDL library if needed, otherwise build
rem as statically linkable (not possible when using SDL)
if "%LIBAROMA_PLATFORM%" == "sdl" (
	set LIBAROMA_ADDITIONAL_LIBS=%LIBAROMA_ADDITIONAL_LIBS% -lSDL
) else if "%LIBAROMA_PLATFORM%" == "sdl2" (
	set LIBAROMA_ADDITIONAL_LIBS=%LIBAROMA_ADDITIONAL_LIBS% -lSDL2
) else (
	set LIBAROMA_APP_STATIC=-static
	rem if platform is Linux and architecture is arm but not neon optimized, disable drm
	if "%LIBAROMA_ARCH%" == "arm" if not "%LIBAROMA_ARCH_APPEND%" == "neon" set LIBAROMA_BUILD_DRM=0
	
	rem enable minui if drm is going to be built
	if !LIBAROMA_BUILD_DRM! == 1 set LIBAROMA_BUILD_MINUI=1
	if !LIBAROMA_BUILD_DRM! == 1 set LIBAROMA_CFLAGS=%LIBAROMA_CFLAGS% -DLIBAROMA_GFX_MINUI=1
)

set "LIBAROMA_ARCH_OPTIMIZATIONS= "
if exist "%LIBAROMA_BASE%\src\aroma\arch\%LIBAROMA_ARCH%\" (
for /f "tokens=*" %%F in ('dir /b /a:-d "%LIBAROMA_BASE%\src\aroma\arch\%LIBAROMA_ARCH%\*.S"') do call set LIBAROMA_ARCH_OPTIMIZATIONS=%%LIBAROMA_ARCH_OPTIMIZATIONS%% "%LIBAROMA_BASE%\src\aroma\arch\%LIBAROMA_ARCH%\%%F"
)
rem MIPS optimizations are in a .c file, not in a .S one
if "%LIBAROMA_ARCH%" == "mips" (
	rem Remember that Windows CMD doesn't support wildcards! Read app.sh for more info.
	rem set LIBAROMA_ARCH_OPTIMIZATIONS=mips\*.c
	set "LIBAROMA_ARCH_OPTIMIZATIONS= "
	for /f "tokens=*" %%F in ('dir /b /a:-d "%LIBAROMA_BASE%\src\aroma\arch\mips\*.c"') do call set LIBAROMA_ARCH_OPTIMIZATIONS=%%LIBAROMA_ARCH_OPTIMIZATIONS%% "%LIBAROMA_BASE%\src\aroma\arch\mips\%%F"
)
rem if targetting SDL we may be building for Windows, which doesn't support some optimizations.
if "%LIBAROMA_PLATFORM%" == "sdl" set "LIBAROMA_ARCH_OPTIMIZATIONS= " && set LIBAROMA_CFLAGS=%LIBAROMA_CFLAGS% -DLIBAROMA_CONFIG_NO_ANDROID_MEM=1
if "%LIBAROMA_PLATFORM%" == "sdl2" set "LIBAROMA_ARCH_OPTIMIZATIONS= " && set LIBAROMA_CFLAGS=%LIBAROMA_CFLAGS% -DLIBAROMA_CONFIG_NO_ANDROID_MEM=1

rem set debug levels
if %LIBAROMA_DEBUG_ENABLED% == 1 (
	if not defined LIBAROMA_CONFIG_DEBUG set LIBAROMA_CONFIG_DEBUG=5
	if not defined LIBAROMA_CONFIG_DEBUG_FILE set LIBAROMA_CONFIG_DEBUG_FILE=2
	set LIBAROMA_FREETYPE_CFLAGS=%LIBAROMA_FREETYPE_CFLAGS% -DFT_DEBUG_LEVEL_ERROR
)

if %LIBAROMA_BUILD_HARFBUZZ% == 1 (
	set LIBAROMA_FREETYPE_CFLAGS=%LIBAROMA_FREETYPE_CFLAGS% -DFT_CONFIG_OPTION_USE_HARFBUZZ
) else set LIBAROMA_CFLAGS=%LIBAROMA_CFLAGS% -DLIBAROMA_CONFIG_TEXT_NOHARFBUZZ

if not defined LIBAROMA_CONFIG_DEBUG_MEMORY set LIBAROMA_CONFIG_DEBUG_MEMORY=0
if not defined LIBAROMA_CONFIG_COMPILER_MESSAGE set LIBAROMA_CONFIG_COMPILER_MESSAGE=1
if not defined LIBAROMA_CONFIG_SHMEMFB set LIBAROMA_CONFIG_SHMEMFB=0

set LIBAROMA_DONE_TEXT=Ready to build libaroma%LIBAROMA_DEBUG_SUFFIX%
set LIBAROMA_DONE_TEXT=%LIBAROMA_DONE_TEXT% targeting %LIBAROMA_PLATFORM% on %LIBAROMA_ARCH%
rem if arch is ARM/x86, add optimization info
if "%LIBAROMA_ARCH%" == "arm" (
	if "%LIBAROMA_ARCH_APPEND%" == "neon" set LIBAROMA_DONE_TEXT=%LIBAROMA_DONE_TEXT% (NEON optimized^)
) else if "%LIBAROMA_ARCH%" == "x86" (
		set LIBAROMA_DONE_TEXT=%LIBAROMA_DONE_TEXT% (SSE optimized^)
) else if "%LIBAROMA_ARCH%" == "x86_64" (
		set LIBAROMA_DONE_TEXT=%LIBAROMA_DONE_TEXT% (SSE optimized^)
)
echo %LIBAROMA_DONE_TEXT%
set "%LIBAROMA_DONE_TEXT="

set LIBAROMA_TARGET_NAME=%LIBAROMA_ARCH%%LIBAROMA_ARCH_APPEND%%LIBAROMA_DEBUG_SUFFIX%

goto :eof

:trim
setlocal enabledelayedexpansion
call :trimhelper %%%1%%
endlocal & set %1=%tempvar:"=%
goto :eof

:trimhelper
set tempvar=%*
goto :eof
