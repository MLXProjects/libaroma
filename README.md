![libaroma-logo](/logo.png?raw=true "Libaroma logo")

# Libaroma

an embedded ui toolkit - originally by Ahmad Amarullah (Indonesia), improved by Michael Jauregui (Argentina)

## What's this?

Written almost entirely in C, libaroma is a library/toolkit which aims to make it easy to build graphical applications for minimal environments (like Linux busybox rootfs) without the need to run an entire window server/manager like Xorg, while having the nice graphics an user would expect.

It also has a SDL compatibility layer, so you can test your apps (if you build on Linux) directly on your desktop or, if you want, even build entire desktop applications using it as UI :)

## Supported platforms

### for building (host)

Linux and Windows

### for running (target)

Linux, Windows (thanks SDL) and QNX

### target architectures

Currently ARM (v6 onwards), x86 and x86_64 are supported. MIPS and other platforms are not tested.

### optimizations

If you target ARM or x86/64, you have the following optional optimizations:

ARM: you can use NEON (available from ARMv7 onwards) to improve drawing operations

x86 and x86_64: you can se SSE to emulate NEON and make drawing operations faster

## Building

read the file BUILDING.md in this page

## Issues/help

If you want to report any issues (or just want some help), please either use the issues tab from Github or email me directly to maicolinux4@gmail.com - feedback is highly appreciated!
