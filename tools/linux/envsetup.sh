#!/bin/sh

export LIBAROMA_GCC="gcc"
export LIBAROMA_GPP="g++"

export LIBAROMA_CFLAGS="-O3 -ftree-vectorize -mvectorize-with-neon-quad -fprefetch-loop-arrays -funsafe-math-optimizations -fopenmp -mfloat-abi=hard -mfpu=neon -D__ARM_HAVE_NEON -DLIBAROMA_CONFIG_OPENMP=1"
export LIBAROMA_DEBUG_CFLAGS="-ggdb -fopenmp -DLIBAROMA_CONFIG_OPENMP=1 -DLIBAROMA_CONFIG_DEBUG_TRACE=3 -mfloat-abi=hard -mfpu=neon -D__ARM_HAVE_NEON"

export LIBAROMA_CONFIG_DEBUG="3"
export LIBAROMA_CONFIG_DEBUG_FILE="2"
export LIBAROMA_CONFIG_DEBUG_MEMORY="0"
export LIBAROMA_CONFIG_COMPILER_MESSAGE="1"
export LIBAROMA_CONFIG_SHMEMFB="0"

bash
