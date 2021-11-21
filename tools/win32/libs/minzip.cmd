@echo off

if not defined LIBAROMA_BASE (
	echo Libaroma environment not set, please run envsetup.cmd first.
	goto :eof
)

if not exist "%LIBAROMA_BASE%\out\libs\%LIBAROMA_TARGET_NAME%\%~n0\" (
	mkdir "%LIBAROMA_BASE%\out\libs\%LIBAROMA_TARGET_NAME%\%~n0"
)
pushd "%LIBAROMA_BASE%\out\libs\%LIBAROMA_TARGET_NAME%\%~n0"

echo Building MinZIP
%LIBAROMA_GCC% -c ^
  -fdata-sections -ffunction-sections -Wl,--gc-sections ^
  -fPIC -DPIC %LIBAROMA_STRIP_OBJECT% -D_GNU_SOURCE -DLOG_NDEBUG ^
  ^
   %LIBAROMA_CFLAGS% ^
	^
	"%LIBAROMA_BASE%\libs\minzip\Hash.c" ^
	"%LIBAROMA_BASE%\libs\minzip\SysUtil.c" ^
	"%LIBAROMA_BASE%\libs\minzip\DirUtil.c" ^
	"%LIBAROMA_BASE%\libs\minzip\Inlines.c" ^
	"%LIBAROMA_BASE%\libs\minzip\Zip.c" ^
	^
  -I"%LIBAROMA_BASE%\libs\mman" ^
  -I"%LIBAROMA_BASE%\libs\selinux\include" ^
  -I"%LIBAROMA_BASE%\libs\zlib\src"

popd
