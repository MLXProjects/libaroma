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
 * Filename		: aroma_platform.h
 * Description : platform header
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 06/04/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_aroma_internal_h__
	#error "Include <aroma_internal.h> instead."
#endif
#ifndef __libaroma_platform_h__
#define __libaroma_platform_h__

#ifdef __cplusplus
extern "C" {
#endif
#ifndef LIBAROMA_PFLINUX_DUMMY_PTHREAD
	#include <pthread.h>	/* pthread_ */
	#include <signal.h>		/* pthread_kill */
#endif

#include <time.h>			/* clock_gettime */
#include <sys/param.h>		/* MIN,MAX */
#include <sys/mman.h>		/* mmap, munmap */
#include <unistd.h>			/* open, close, unlink, usleep */
#include <fcntl.h>			/* open flags */

/* shmem prefix */
#ifndef LIBAROMA_CONFIG_SHMEM_PREFIX
	#define LIBAROMA_CONFIG_SHMEM_PREFIX "/libaromashm-"
#endif

/* Android */
#ifdef ANDROID
#define LIBAROMA_CONFIG_OS "linux/android"
/* android wrapper for shm_* */
#define shm_open open
#define shm_unlink unlink
#else
#define LIBAROMA_CONFIG_OS "linux/gnu"
#endif

/*
 * platform flags
 */

#define LIBAROMA_PLATFORM_HAS_SHMEM	1
#define LIBAROMA_PLATFORM_HAS_MMAP	1
#define LIBAROMA_PLATFORM_HAS_FD	1

/*
 * common platform wrapper
 */
static inline int libaroma_unlink(char *filename){
	return unlink(filename);
}
static inline void libaroma_sleep(int ms){
	usleep(ms*1000);
}

/*
 * get tick count
 */
static inline long libaroma_tick(){
	struct timespec now;
	if (clock_gettime(CLOCK_MONOTONIC, &now)) {
		return 0;
	}
	return ((long) (now.tv_sec * 1000 + now.tv_nsec / 1000000));
}

/*
 * MUTEX - NEED MULTICORE THREAD SAFE
 */
#ifndef LIBAROMA_PFLINUX_DUMMY_PTHREAD
	#ifdef LIBAROMA_CONFIG_OPENMP
		#include <omp.h>
		#define LIBAROMA_MUTEX omp_nest_lock_t
		#define libaroma_mutex_init(x) omp_init_nest_lock(&x)
		#define libaroma_mutex_free(x) omp_destroy_nest_lock(&x)
		#define libaroma_mutex_lock(x) omp_set_nest_lock(&x)
		#define libaroma_mutex_unlock(x) omp_unset_nest_lock(&x)
	#else
		#define LIBAROMA_MUTEX pthread_mutex_t
		#define libaroma_mutex_init(x) pthread_mutex_init(&x,NULL)
		#define libaroma_mutex_free(x) pthread_mutex_destroy(&x)
		#define libaroma_mutex_lock(x) pthread_mutex_lock(&x)
		#define libaroma_mutex_unlock(x) pthread_mutex_unlock(&x)
	#endif

	/*
	 * THREADS
	 */
	typedef pthread_t LIBAROMA_THREAD;
	#define libaroma_thread_create(th,cb,cookie) \
		pthread_create(th,NULL,cb,cookie)
	/* recommended */
	#define libaroma_thread_join(th) pthread_join(th,NULL)
	/* optional */
	#define libaroma_thread_detach(th) 			pthread_detach(th)
	#define libaroma_thread_kill(th) 			pthread_kill(th,0)
	#define libaroma_thread_kill_ex(th, sig) 	pthread_kill(th, sig)
	#define libaroma_thread_cancel(th)			pthread_cancel(th)
	static inline void libaroma_thread_set_hiprio(LIBAROMA_THREAD t){
		struct sched_param params;
		params.sched_priority = sched_get_priority_max(SCHED_FIFO);
		pthread_setschedparam(t, SCHED_FIFO, &params);
	}

	/*
	 * CONDITION & MUTEX CONDITION - NOT NEED MULTICORE THREADSAFE
	 */
	#define LIBAROMA_COND_MUTEX pthread_mutex_t
	#define LIBAROMA_COND pthread_cond_t
	#define libaroma_cond_wait(c,m) pthread_cond_wait(c,m)
	#define libaroma_cond_signal(c) pthread_cond_broadcast(c)
	#define libaroma_cond_lock(m) pthread_mutex_lock(m)
	#define libaroma_cond_unlock(m) pthread_mutex_unlock(m)
	void libaroma_cond_init(LIBAROMA_COND * cond, LIBAROMA_COND_MUTEX * mutex);
	void libaroma_cond_free(LIBAROMA_COND * cond, LIBAROMA_COND_MUTEX * mutex);

#else
	#define LIBAROMA_MUTEX voidp
	#define libaroma_mutex_init(x)
	#define libaroma_mutex_free(x)
	#define libaroma_mutex_lock(x)
	#define libaroma_mutex_unlock(x)
	typedef voidp LIBAROMA_THREAD;
	#define libaroma_thread_create(th,cb,cookie)
	#define libaroma_thread_join(th)
	#define libaroma_thread_detach(th)
	#define libaroma_thread_kill(th)
	static inline void libaroma_thread_set_hiprio(LIBAROMA_THREAD t){
	}
	/*
	 * CONDITION & MUTEX CONDITION - NOT NEED MULTICORE THREADSAFE
	 */
	#define LIBAROMA_COND_MUTEX voidp
	#define LIBAROMA_COND voidp
	#define libaroma_cond_wait(c,m)
	#define libaroma_cond_signal(c)
	#define libaroma_cond_lock(m)
	#define libaroma_cond_unlock(m)
	void libaroma_cond_init(LIBAROMA_COND * cond, LIBAROMA_COND_MUTEX * mutex);
	void libaroma_cond_free(LIBAROMA_COND * cond, LIBAROMA_COND_MUTEX * mutex);
#endif

/*
 * Pointer coords (if available)
 */
extern int LINUXHIDRV_get_mouse_x();
extern int LINUXHIDRV_get_mouse_y();

#ifdef __cplusplus
}
#endif
#endif /* __libaroma_platform_h__ */

