cmake_minimum_required(VERSION 3.4)
project(libmlx VERSION 1.1.0 LANGUAGES C)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# useful paths
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# set sources
set(LIB_SRC ${LIB_DIR}/lib.c)

option(LIBMLX_USEAF "" ON)
option(TESTS "Build test binaries" ON)

if(LIBMLX_USEAF EQUAL YES)
set(LIB_SRC ${LIB_SRC} ${LIB_DIR}/af/af.c)
else()
set(LIB_SRC ${LIB_SRC} ${LIB_DIR}/asd/asd.c)
endif()

# Create our library target
add_library(mlx SHARED ${LIB_SRC})

# set lib props
set_target_properties(mlx
	PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# set include directories
target_include_directories(mlx
PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Set output directory for the libraries & interactive binary. Tests go in build directory, except on windows because windows sucks my balls
set(OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/out)
set_target_properties(mlx
PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
)

# compile test?
if(TESTS)
	# test paths
	set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)
	set(TEST_SRC ${TEST_DIR}/main.c)
	# add executable target
	add_executable(testapp ${TEST_SRC})
	target_link_libraries(testapp mlx)
endif()