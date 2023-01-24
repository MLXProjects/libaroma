/********************************************************************[libaroma]*
 * Copyright (C) 2011-2015 Ahmad Amarullah (http://amarullz.com/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *			http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *______________________________________________________________________________
 *
 * Filename		: aroma_internal.h
 * Description : libaroma internal header
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 06/04/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_aroma_internal_h__
#define __libaroma_aroma_internal_h__

/* include main header */
#include <aroma.h>

/* some fallbacks */
#ifndef MIN
#define MIN(a, b) ((a<b)?a:b)
#endif
#ifndef MAX
#define MAX(a, b) ((a>b)?a:b)
#endif

/*
 * Libaroma version configurations
 */
#define LIBAROMA_CONFIG_NAME			"libaroma"
#define LIBAROMA_CONFIG_VERSION_MAJOR	1
#define LIBAROMA_CONFIG_VERSION_MINOR	1
#define LIBAROMA_CONFIG_VERSION_MICRO	0
#define LIBAROMA_CONFIG_VERSION_BUILD	"230110"
#define LIBAROMA_CONFIG_CODENAME		"Kinanthi"
#define LIBAROMA_CONFIG_YEAR			"2011-2023"
#define LIBAROMA_CONFIG_AUTHOR			"Ahmad Amarullah"

#define LIBAROMA_FB_INIT_FUNCTION		libaroma_fb_driver_init
#define LIBAROMA_HID_INIT_FUNCTION		libaroma_hid_driver_init

#ifdef LIBAROMA_CONFIG_HICOLOR_BIT
#if LIBAROMA_CONFIG_HICOLOR_BIT > 0
	#define LIBAROMA_CONFIG_USE_HICOLOR_BIT
#endif
#endif /* LIBAROMA_CONFIG_HICOLOR_BIT */

/* currently, runtime monitor only supports Linux */
#ifdef __linux__
#define LIBAROMA_RUNTIME_MONITOR
#else
#ifdef LIBAROMA_RUNTIME_MONITOR
#undef LIBAROMA_RUNTIME_MONITOR
#endif /* LIBAROMA_RUNTIME_MONITOR */
#endif /* __linux__ */

/* standard c headers */
#include <string.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>

/* debug, platform & graph engine */
#include "debug/debug.h"
#include <aroma_plat.h>
#include "graph/engine.h"
#include "ui/internal.h"

/* some control functions/callbacks don't need all their parameters */
#ifdef __clang__
	#pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined(__GNUC__)
	#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif /* __clang__ */

#endif /* __libaroma_aroma_internal_h__ */
