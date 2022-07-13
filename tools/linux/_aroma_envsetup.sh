if [ -z "${_LIBAROMA_ENVSETUP_ROOT}" ]; then
	echo Please use envsetup at source root
	exit
fi

export LIBAROMA_BASE="${_LIBAROMA_ENVSETUP_ROOT}"

#thanks a lot @AKIANONYMOUS for writing this!
#start of config parser
_config_file="${LIBAROMA_BASE}/config-linux.txt"

# check if the config file is accessible
[ -r "${_config_file}" ] || {
	echo "WARNING: config file ( ${_config_file} ) is not readable, using defaults!"
}

# Setting 'IFS' tells 'read' where to split the string.
while IFS='=' read -r key val; do
	# Skip Lines starting with '#'
	# Also skip lines if key and val variable is empty
	{ [ -n "${key}" ] && [ -n "${val}" ] && [ -n "${key##\#*}" ] ;} || continue

	# trim all trailing whitespace/double quotes
	key="${key#${key%%[![:space:]]*}}"
	val="${val#${val%%[![:space:]]*}}"
	key="${key#${key%%[!\"]*}}"
	val="${val#${val%%[!\"]*}}"
	#key="${key#\"}"
	#val="${val#\"}"

	# trim all leading whitespace/double quotes
	key="${key%${key##*[![:space:]]}}"
	val="${val%${val##*[![:space:]]}}"
	key="${key%${key##*[!\"]}}"
	val="${val%${val##*[!\"]}}"
	#key="${key%\"}"
	#val="${val%\"}"

	# MLX: try to remove carriage return if present
	val="${val%%$(printf '\r')}"

	# '$key' stores the key and '$val' stores the value.
	# Throw a warning if cannot export the variable
	export "${key}=${val}" 2>/dev/null ||
		echo "Warning: ${key} is not a valid config name."
done < "${_config_file}"
#end of config parser

if [ -z "${LIBAROMA_DEBUG_ENABLED}" ]; then LIBAROMA_DEBUG_ENABLED=0
fi
if [ "$1" = "d" ]; then
	LIBAROMA_DEBUG_ENABLED=1
fi

if [ -z "${LIBAROMA_ARCH}" ]; then
	 LIBAROMA_ARCH="arm"
fi
if [ -z "${LIBAROMA_ARCH_APPEND}" ]; then
	 LIBAROMA_ARCH_APPEND=""
fi
if [ -z "${LIBAROMA_PLATFORM}" ]; then
	 LIBAROMA_PLATFORM="linux"
fi

if [ -z "${LIBAROMA_GCC}" ]; then
	 LIBAROMA_GCC="${LIBAROMA_TOOLCHAIN_PREFIX}gcc"
fi
if [ -z "${LIBAROMA_GPP}" ]; then
	 LIBAROMA_GPP="${LIBAROMA_TOOLCHAIN_PREFIX}g++"
fi

# some default values
if [ -z "${LIBAROMA_BUILD_DRM}" ]; then
	 LIBAROMA_BUILD_DRM=0
fi
if [ -z "${LIBAROMA_BUILD_HARFBUZZ}" ]; then
	 LIBAROMA_BUILD_HARFBUZZ=1
fi
if [ -z "${LIBAROMA_BUILD_JPEG}" ]; then
	 LIBAROMA_BUILD_JPEG=0
fi
if [ -z "${LIBAROMA_BUILD_MINUI}" ]; then
	 LIBAROMA_BUILD_MINUI=0
fi
if [ -z "${LIBAROMA_BUILD_RUNTIME}" ]; then
	 LIBAROMA_BUILD_RUNTIME=0
fi
if [ -z "${LIBAROMA_CONFIG_DEBUG}" ]; then
	 LIBAROMA_CONFIG_DEBUG=3
fi
if [ -z "${LIBAROMA_CONFIG_DEBUG_FILE}" ]; then
	 LIBAROMA_CONFIG_DEBUG_FILE=0
fi

if [ -z "${LIBAROMA_CONFIG_OPENMP}" ]; then
	 LIBAROMA_CONFIG_OPENMP=1
fi

if [ "${LIBAROMA_CONFIG_OPENMP}" = "1" ]; then
	LIBAROMA_CFLAGS="${LIBAROMA_CFLAGS} -fopenmp -DLIBAROMA_CONFIG_OPENMP=1"
fi

if [ -z "${LIBAROMA_COMPILER_WARNINGS}" ]; then
	 LIBAROMA_COMPILER_WARNINGS=0
fi

if [ "${LIBAROMA_COMPILER_WARNINGS}" = "2" ]; then
	LIBAROMA_CFLAGS="${LIBAROMA_CFLAGS} -Wextra"
elif [ "${LIBAROMA_COMPILER_WARNINGS}" = "1" ]; then
	 LIBAROMA_CFLAGS="${LIBAROMA_CFLAGS} -Wall"
else
	LIBAROMA_CFLAGS="${LIBAROMA_CFLAGS} -w"
fi

if [ -z "${LIBAROMA_COMPILER_ERROR_ON_WARN}" ]; then
	 LIBAROMA_COMPILER_ERROR_ON_WARN=0
fi
if [ "${LIBAROMA_COMPILER_ERROR_ON_WARN}" = "1" ]; then
	 LIBAROMA_CFLAGS="${LIBAROMA_CFLAGS} -Werror"
fi

# if building for android, revert platform to linux
# and enable android in cflags
if [ "${LIBAROMA_PLATFORM}" = "android" ]; then
	LIBAROMA_CFLAGS="${LIBAROMA_CFLAGS} -DANDROID=1 -D__ANDROID__"
	LIBAROMA_PLATFORM="linux"
fi

# if debug is enabled, use minimal cflags with debug.
# otherwise, use optimizations and strip output
if [ "${LIBAROMA_DEBUG_ENABLED}" = "1" ]; then
	LIBAROMA_DEBUG_SUFFIX="-debug"
	LIBAROMA_CFLAGS="${LIBAROMA_CFLAGS} -g -ggdb -DLIBAROMA_CONFIG_DEBUG_TRACE=3 -D_GLIBCXX_DEBUG_PEDANTIC -D_GLIBCXX_DEBUG"
else
	LIBAROMA_DEBUG_SUFFIX="-release"
	LIBAROMA_CFLAGS="${LIBAROMA_CFLAGS} -s -O3 -ftree-vectorize -fprefetch-loop-arrays -funsafe-math-optimizations"
	LIBAROMA_STRIP_OBJECT="-Wl,-s"
fi

# if targetting sdl/sdl2, use init helper (SDL window creation and input loop are done in same thread)
# This is needed because SDL doesn't seem to like handling requests from multiple threads :/
if [ "${LIBAROMA_PLATFORM}" = "sdl" ] || [ "${LIBAROMA_PLATFORM}" = "sdl2" ]; then 
	LIBAROMA_CFLAGS="${LIBAROMA_CFLAGS} -DLIBAROMA_INIT_HELPER"
fi

# enable architecture optimizations
if [ "${LIBAROMA_ARCH}" = "arm" ] && [ "${LIBAROMA_ARCH_APPEND}" = "neon" ]; then
	LIBAROMA_CFLAGS="${LIBAROMA_CFLAGS} -mfloat-abi=hard -mfpu=neon -D__ARM_HAVE_NEON -mvectorize-with-neon-quad"
elif [ "${LIBAROMA_ARCH}" = "x86" ] || [ "${LIBAROMA_ARCH}" = "x86_64" ]; then
	LIBAROMA_CFLAGS="${LIBAROMA_CFLAGS} -D__i386"
fi

if [ "${LIBAROMA_ARCH_APPEND}" != "neon" ] || [ "${LIBAROMA_BUILD_JPEG}" = "0" ]; then
		LIBAROMA_CFLAGS="${LIBAROMA_CFLAGS} -DLIBAROMA_CONFIG_NOJPEG=1"
fi

# link with SDL library if needed, otherwise build
# as statically linkable (not possible when using SDL)
if [ "${LIBAROMA_PLATFORM}" = "sdl" ]; then
	LIBAROMA_ADDITIONAL_LIBS="${LIBAROMA_ADDITIONAL_LIBS} -lSDL"
elif [ "${LIBAROMA_PLATFORM}" = "sdl2" ]; then
	LIBAROMA_ADDITIONAL_LIBS="${LIBAROMA_ADDITIONAL_LIBS} -lSDL2"
else
	LIBAROMA_APP_STATIC="-static"
	# if platform is Linux and architecture is arm but not neon optimized, disable drm
	if [ "${LIBAROMA_ARCH}" = "arm" ] && [ "${LIBAROMA_ARCH_APPEND}" != "neon" ]; then
		LIBAROMA_BUILD_DRM=0
	fi

	# enable minui if drm is going to be built
	if [ "${LIBAROMA_BUILD_DRM}" = "1" ]; then
		LIBAROMA_BUILD_MINUI=1
		LIBAROMA_CFLAGS="${LIBAROMA_CFLAGS} -DLIBAROMA_GFX_MINUI=1"
	fi
fi

if [ -d "${LIBAROMA_BASE}/src/aroma/arch/${LIBAROMA_ARCH}" ]; then
	LIBAROMA_ARCH_OPTIMIZATIONS="${LIBAROMA_ARCH}/*.S"
fi

# MIPS optimizations are in a .c file, not in a .S one
if [ "${LIBAROMA_ARCH}" = "mips" ]; then
	LIBAROMA_ARCH_OPTIMIZATIONS="mips/*.c"
fi

# set debug levels
if [ "${LIBAROMA_DEBUG_ENABLED}" = "1" ]; then
	if [ -z "${LIBAROMA_CONFIG_DEBUG}" ]; then
		LIBAROMA_CONFIG_DEBUG=5
	fi
	if [ -z "${LIBAROMA_CONFIG_DEBUG_FILE}" ]; then
		LIBAROMA_CONFIG_DEBUG_FILE=2
	fi
	LIBAROMA_FREETYPE_CFLAGS="${LIBAROMA_FREETYPE_CFLAGS} -DFT_DEBUG_LEVEL_ERROR"
fi

if [ "${LIBAROMA_BUILD_HARFBUZZ}" = "1" ]; then
	LIBAROMA_FREETYPE_CFLAGS="${LIBAROMA_FREETYPE_CFLAGS} -DFT_CONFIG_OPTION_USE_HARFBUZZ"
else
	LIBAROMA_CFLAGS="${LIBAROMA_CFLAGS} -DLIBAROMA_CONFIG_TEXT_NOHARFBUZZ"
fi

if [ -z "${LIBAROMA_CONFIG_DEBUG_MEMORY}" ]; then
	 LIBAROMA_CONFIG_DEBUG_MEMORY=0
fi
if [ -z "${LIBAROMA_CONFIG_COMPILER_MESSAGE}" ]; then
	 LIBAROMA_CONFIG_COMPILER_MESSAGE=1
fi
if [ -z "${LIBAROMA_CONFIG_SHMEMFB}" ]; then
	 LIBAROMA_CONFIG_SHMEMFB=0
fi

LIBAROMA_TARGET_NAME="${LIBAROMA_ARCH}${LIBAROMA_ARCH_APPEND}${LIBAROMA_DEBUG_SUFFIX}"

#export options to make them available to other scripts
export LIBAROMA_ADDITIONAL_LIBS
export LIBAROMA_APP_STATIC
export LIBAROMA_ARCH
export LIBAROMA_ARCH_APPEND
export LIBAROMA_ARCH_OPTIMIZATIONS
export LIBAROMA_BUILD_DRM
export LIBAROMA_BUILD_HARFBUZZ
export LIBAROMA_BUILD_JPEG
export LIBAROMA_BUILD_MINUI
export LIBAROMA_BUILD_RUNTIME
export LIBAROMA_CFLAGS
export LIBAROMA_COMPILER_WARNINGS
export LIBAROMA_COMPILER_ERROR_ON_WARN
export LIBAROMA_CONFIG_COMPILER_MESSAGE
export LIBAROMA_CONFIG_DEBUG
export LIBAROMA_CONFIG_DEBUG_FILE
export LIBAROMA_CONFIG_DEBUG_MEMORY
export LIBAROMA_CONFIG_OPENMP
export LIBAROMA_CONFIG_SHMEMFB
export LIBAROMA_DEBUG_ENABLED
export LIBAROMA_DEBUG_SUFFIX
export LIBAROMA_FREETYPE_CFLAGS
export LIBAROMA_GCC
export LIBAROMA_GDB
export LIBAROMA_GPP
export LIBAROMA_PLATFORM
export LIBAROMA_STRIP_OBJECT
export LIBAROMA_TARGET_NAME
export PATH="${PATH}:${LIBAROMA_BASE}/tools/linux"

LIBAROMA_DONE_TEXT="Ready to build libaroma${LIBAROMA_DEBUG_SUFFIX} targeting ${LIBAROMA_PLATFORM} on ${LIBAROMA_ARCH}"

# if arch is ARM/x86, add optimization info
if [ "${LIBAROMA_ARCH}" = "arm" ]; then
	if [ "${LIBAROMA_ARCH_APPEND}" = "neon" ]; then
		LIBAROMA_DONE_TEXT="${LIBAROMA_DONE_TEXT} (NEON optimized)"
	fi
elif [ "${LIBAROMA_ARCH}" = "x86" ] || [ "${LIBAROMA_ARCH}" = "x86_64" ]; then
	LIBAROMA_DONE_TEXT="${LIBAROMA_DONE_TEXT} (SSE optimized)"
fi

echo "${LIBAROMA_DONE_TEXT}"
