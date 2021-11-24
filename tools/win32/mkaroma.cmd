@echo off

if not defined LIBAROMA_BASE (
	echo Libaroma environment not set, please run envsetup.cmd first.
	goto :eof
)

if "%1"=="/?" ( goto :showhelp
) else if "%1"=="-h" ( goto :showhelp
) else if "%1"=="--help" ( goto :showhelp
)

if not "%1"=="nd" if not "%1"=="nodetect" call :dep_detect

if not exist "%LIBAROMA_BASE%\out\libs\%LIBAROMA_TARGET_NAME%\aroma\" mkdir "%LIBAROMA_BASE%\out\libs\%LIBAROMA_TARGET_NAME%\aroma"
pushd "%LIBAROMA_BASE%\out\libs\%LIBAROMA_TARGET_NAME%\aroma"

if %LIBAROMA_BUILD_DRM% == 1 (
echo Building MinUI wrapper
%LIBAROMA_GCC% -c ^
  -fdata-sections -ffunction-sections -Wl,--gc-sections ^
  -lstdc++ ^
  -fPIC -DPIC ^
 ^
  %LIBAROMA_CFLAGS% ^
	^
	"%LIBAROMA_BASE%\src\contrib\platform\%LIBAROMA_PLATFORM%\aroma_minui.cpp" ^
	^
  -I"%LIBAROMA_BASE%\libs\minui" ^
  -I"%LIBAROMA_BASE%\src\contrib\platform\%LIBAROMA_PLATFORM%\include"
)

echo Building Libaroma%LIBAROMA_DEBUG_SUFFIX%
%LIBAROMA_GCC% -c ^
  -fdata-sections -ffunction-sections -Wl,--gc-sections ^
  -lstdc++ %LIBAROMA_STRIP_OBJECT% ^
  -fPIC -DPIC -DNOGDI^
 ^
  %LIBAROMA_CFLAGS% ^
 ^
  -DLIBAROMA_CONFIG_DEBUG=%LIBAROMA_CONFIG_DEBUG%% ^
  -DLIBAROMA_CONFIG_DEBUG_FILE=%LIBAROMA_CONFIG_DEBUG_FILE% ^
  -DLIBAROMA_CONFIG_DEBUG_MEMORY=%LIBAROMA_CONFIG_DEBUG_MEMORY% ^
  -DLIBAROMA_CONFIG_COMPILER_MESSAGE=%LIBAROMA_CONFIG_COMPILER_MESSAGE% ^
  -DLIBAROMA_CONFIG_SHMEMFB=%LIBAROMA_CONFIG_SHMEMFB% ^
  ^
	"%LIBAROMA_BASE%\src\aroma\debug\compiler_message.c" ^
	^
	"%LIBAROMA_BASE%\src\aroma\aroma.c" ^
	"%LIBAROMA_BASE%\src\aroma\version.c" ^
	^
	%LIBAROMA_ARCH_OPTIMIZATIONS% ^
	^
	"%LIBAROMA_BASE%\src\contrib\platform\%LIBAROMA_PLATFORM%\fb_driver.c" ^
	"%LIBAROMA_BASE%\src\contrib\platform\%LIBAROMA_PLATFORM%\hid_driver.c" ^
	"%LIBAROMA_BASE%\src\contrib\platform\%LIBAROMA_PLATFORM%\platform.c" ^
	^
	"%LIBAROMA_BASE%\src\aroma\controls\ctl_bar.c" ^
	"%LIBAROMA_BASE%\src\aroma\controls\ctl_button.c" ^
	"%LIBAROMA_BASE%\src\aroma\controls\ctl_fragment.c" ^
	"%LIBAROMA_BASE%\src\aroma\controls\ctl_image.c" ^
	"%LIBAROMA_BASE%\src\aroma\controls\ctl_label.c" ^
	"%LIBAROMA_BASE%\src\aroma\controls\ctl_list.c" ^
	"%LIBAROMA_BASE%\src\aroma\controls\ctl_pager.c" ^
	"%LIBAROMA_BASE%\src\aroma\controls\ctl_progress.c" ^
	"%LIBAROMA_BASE%\src\aroma\controls\ctl_scroll.c" ^
	"%LIBAROMA_BASE%\src\aroma\controls\ctl_slider.c" ^
	"%LIBAROMA_BASE%\src\aroma\controls\ctl_tabs.c" ^
	"%LIBAROMA_BASE%\src\aroma\controls\ctl_toast.c" ^
	"%LIBAROMA_BASE%\src\aroma\controls\listitem\listitem_caption.c" ^
	"%LIBAROMA_BASE%\src\aroma\controls\listitem\listitem_check.c" ^
	"%LIBAROMA_BASE%\src\aroma\controls\listitem\listitem_divider.c" ^
	"%LIBAROMA_BASE%\src\aroma\controls\listitem\listitem_image.c" ^
	"%LIBAROMA_BASE%\src\aroma\controls\listitem\listitem_menu.c" ^
	"%LIBAROMA_BASE%\src\aroma\controls\listitem\listitem_option.c" ^
	"%LIBAROMA_BASE%\src\aroma\controls\listitem\listitem_text.c" ^
	"%LIBAROMA_BASE%\src\aroma\debug\memory_tracking.c" ^
	"%LIBAROMA_BASE%\src\aroma\graph\artworker.c" ^
	"%LIBAROMA_BASE%\src\aroma\graph\canvas.c" ^
	"%LIBAROMA_BASE%\src\aroma\graph\fb.c" ^
	"%LIBAROMA_BASE%\src\aroma\graph\image.c" ^
	"%LIBAROMA_BASE%\src\aroma\graph\jpeg.c" ^
	"%LIBAROMA_BASE%\src\aroma\graph\png.c" ^
	"%LIBAROMA_BASE%\src\aroma\graph\svg.c" ^
	"%LIBAROMA_BASE%\src\aroma\graph\text.c" ^
	"%LIBAROMA_BASE%\src\aroma\graph\draw\blur.c" ^
	"%LIBAROMA_BASE%\src\aroma\graph\draw\commondraw.c" ^
	"%LIBAROMA_BASE%\src\aroma\graph\draw\filters.c" ^
	"%LIBAROMA_BASE%\src\aroma\graph\draw\gradient.c" ^
	"%LIBAROMA_BASE%\src\aroma\graph\draw\path.c" ^
	"%LIBAROMA_BASE%\src\aroma\graph\draw\scale.c" ^
	"%LIBAROMA_BASE%\src\aroma\graph\engine\alignblt.c" ^
	"%LIBAROMA_BASE%\src\aroma\graph\engine\alpha.c" ^
	"%LIBAROMA_BASE%\src\aroma\graph\engine\blt.c" ^
	"%LIBAROMA_BASE%\src\aroma\graph\engine\color.c" ^
	"%LIBAROMA_BASE%\src\aroma\graph\engine\dither.c" ^
	"%LIBAROMA_BASE%\src\aroma\hid\hid.c" ^
	"%LIBAROMA_BASE%\src\aroma\hid\messages.c" ^
	"%LIBAROMA_BASE%\src\aroma\ui\color_manager.c" ^
	"%LIBAROMA_BASE%\src\aroma\ui\control.c" ^
	"%LIBAROMA_BASE%\src\aroma\ui\dialog.c" ^
	"%LIBAROMA_BASE%\src\aroma\ui\window.c" ^
	"%LIBAROMA_BASE%\src\aroma\ui\window_layer.c" ^
	"%LIBAROMA_BASE%\src\aroma\ui\window_manager.c" ^
	"%LIBAROMA_BASE%\src\aroma\utils\array.c" ^
	"%LIBAROMA_BASE%\src\aroma\utils\json.c" ^
	"%LIBAROMA_BASE%\src\aroma\utils\minutf8.c" ^
	"%LIBAROMA_BASE%\src\aroma\utils\motions.c" ^
	"%LIBAROMA_BASE%\src\aroma\utils\runtime.c" ^
	"%LIBAROMA_BASE%\src\aroma\utils\stream.c" ^
	"%LIBAROMA_BASE%\src\aroma\utils\strings.c" ^
	"%LIBAROMA_BASE%\src\aroma\utils\system.c" ^
	"%LIBAROMA_BASE%\src\aroma\utils\time.c" ^
	"%LIBAROMA_BASE%\src\aroma\utils\xml.c" ^
	"%LIBAROMA_BASE%\src\aroma\utils\zip.c" ^
	^
  -I"%LIBAROMA_BASE%\include" ^
  -I"%LIBAROMA_BASE%\src\contrib\platform\%LIBAROMA_PLATFORM%\include" ^
  -I"%LIBAROMA_BASE%\src" ^
  -I"%LIBAROMA_BASE%\libs\zlib\src" ^
  -I"%LIBAROMA_BASE%\libs\freetype\builds" ^
  -I"%LIBAROMA_BASE%\libs\freetype\include" ^
  -I"%LIBAROMA_BASE%\libs\minzip" ^
  -I"%LIBAROMA_BASE%\libs\mman" ^
  -I"%LIBAROMA_BASE%\libs\selinux\include" ^
  -I"%LIBAROMA_BASE%\libs\png" ^
  -I"%LIBAROMA_BASE%\libs\jpeg" ^
  -I"%LIBAROMA_BASE%\libs\harfbuzz\src" ^
  -I"%LIBAROMA_BASE%\libs\ucdn"

popd

goto :eof

:dep_detect
setlocal enabledelayedexpansion
for %%L in (drm freetype hb jpeg minui minzip png squirrel ucdn zlib) do (
	rem if folder doesn't exist because it 's not needed, ignore the warning
	if not exist "%LIBAROMA_BASE%\out\libs\%LIBAROMA_TARGET_NAME%\%%L\" (
		set should_warn=1
		if %%L == drm if %LIBAROMA_BUILD_DRM% == 0 set should_warn=0
		if %%L == hb if %LIBAROMA_BUILD_HARFBUZZ% == 0 set should_warn=0
		if %%L == jpeg if %LIBAROMA_BUILD_JPEG% == 0 set should_warn=0
		if %%L == minui if %LIBAROMA_BUILD_MINUI% == 0 set should_warn=0
		if %%L == squirrel if %LIBAROMA_BUILD_RUNTIME% == 0 set should_warn=0
		if !should_warn! == 1 (
			rem if library %%L is not found, try to build it
			call "%LIBAROMA_BASE%\tools\win32\mkalibs.cmd" %%L
		)
	)
)
endlocal
goto :eof

:showhelp
call %LIBAROMA_BASE%\tools\win32\aromahelp.cmd %~n0
goto :eof
