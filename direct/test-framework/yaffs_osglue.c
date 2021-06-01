/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2018 Aleph One Ltd.
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * Example OS glue functions for running on a Linux/POSIX system.
 */

#include "yaffscfg.h"
#include "yaffs_guts.h"
#include "yaffsfs.h"
#include "yaffs_trace.h"
#include <assert.h>

#include <errno.h>
#include <unistd.h>

/*
 * yaffsfs_SetError() and yaffsfs_GetError()
 * Do whatever to set the system error.
 * yaffsfs_GetError() just fetches the last error.
 */

static int yaffsfs_lastError;

void yaffsfs_SetError(int err)
{
	//Do whatever to set error
	yaffsfs_lastError = err;
	errno = err;
}

int yaffsfs_GetLastError(void)
{
	return yaffsfs_lastError;
}

/*
 * yaffsfs_CheckMemRegion()
 * Check that access to an address is valid.
 * This can check memory is in bounds and is writable etc.
 *
 * Returns 0 if ok, negative if not.
 */
int yaffsfs_CheckMemRegion(const void *addr, size_t size, int write_request)
{
	(void) size;
	(void) write_request;

	if(!addr)
		return -1;
	return 0;
}

/*
 * yaffsfs_Lock()
 * yaffsfs_Unlock()
 * A single mechanism to lock and unlock yaffs. Hook up to a mutex or whatever.
 * Here are two examples, one using POSIX pthreads, the other doing nothing.
 *
 * If we use pthreads then we also start a background gc thread.
 */

#if 1

#include <pthread.h>

static pthread_mutex_t mutex1;
static pthread_t bc_gc_thread;

void yaffsfs_Lock(void)
{
	pthread_mutex_lock( &mutex1 );
}

void yaffsfs_Unlock(void)
{
	pthread_mutex_unlock( &mutex1 );
}

static void *bg_gc_func(void *dummy)
{
	struct yaffs_dev *dev;
	int urgent = 0;
	int result;
	int next_urgent;

	(void)dummy;

	/* Sleep for a bit to allow start up */
	sleep(2);


	while (1) {
		/* Iterate through devices, do bg gc updating ungency */
		yaffs_dev_rewind();
		next_urgent = 0;

		while ((dev = yaffs_next_dev()) != NULL) {
			result = yaffs_do_background_gc_reldev(dev, urgent);

			/* result is 1 if more than half the free space is
			 * erased.
			 * If less than half the free space is erased then it is
			 * worth doing another background_gc operation sooner.
			 */
			if (result == 0)
				next_urgent = 1;
		}

		urgent = next_urgent;

		if (next_urgent)
			sleep(1);
		else
			sleep(5);
	}

	/* Don't ever return. */
	return NULL;
}

void yaffsfs_LockInit(void)
{
	/* Initialise lock */
	pthread_mutex_init(&mutex1, NULL);

	/* Sneak in starting a background gc thread too */
	pthread_create(&bc_gc_thread, NULL, bg_gc_func, NULL);
}

#else

void yaffsfs_Lock(void)
{
}

void yaffsfs_Unlock(void)
{
}

void yaffsfs_LockInit(void)
{
}
#endif

/*
 * yaffsfs_CurrentTime() retrns a 32-bit timestamp.
 *
 * Can return 0 if your system does not care about time.
 */

u32 yaffsfs_CurrentTime(void)
{
	return time(NULL);
}


/*
 * yaffsfs_malloc()
 * yaffsfs_free()
 *
 * Functions to allocate and free memory.
 */


static unsigned malloc_high_water;
static unsigned malloc_currently_allocated;

#ifdef CONFIG_YAFFS_MONITOR_MALLOC

#include <malloc.h>
static void yaffsfs_malloc_add(void *ptr)
{
	unsigned this_size = malloc_usable_size(ptr);
	malloc_currently_allocated += this_size;

	if (malloc_currently_allocated > malloc_high_water)
		malloc_high_water = malloc_currently_allocated;
}

static void yaffsfs_malloc_remove(void *ptr)
{
	unsigned this_size = malloc_usable_size(ptr);
	malloc_currently_allocated -= this_size;
}
#else
static void yaffsfs_malloc_add(void *ptr)
{
	(void)ptr;
}

static void yaffsfs_malloc_remove(void *ptr)
{
	(void)ptr;
}
#endif

void yaffsfs_get_malloc_values(unsigned *current, unsigned *high_water)
{
	if (current)
		*current = malloc_currently_allocated;
	if(high_water)
		*high_water = malloc_high_water;
}


#ifdef CONFIG_YAFFS_TEST_MALLOC

static int yaffs_kill_alloc = 0;
static size_t total_malloced = 0;
static size_t malloc_limit = 0 & 6000000;

void *yaffsfs_malloc(size_t size)
{
	void * this;
	if(yaffs_kill_alloc)
		return NULL;
	if(malloc_limit && malloc_limit <(total_malloced + size) )
		return NULL;

	this = malloc(size);
	if(this)
		total_malloced += size;
	yaffsfs_malloc_add(this);
	return this;
}

#else

void *yaffsfs_malloc(size_t size)
{
	void *this = malloc(size);
	yaffsfs_malloc_add(this);
	return this;
}

#endif




void yaffsfs_free(void *ptr)
{
	yaffsfs_malloc_remove(ptr);
	free(ptr);
}

void yaffsfs_OSInitialisation(void)
{
	yaffsfs_LockInit();
}

/*
 * yaffs_bug_fn()
 * Function to report a bug.
 */

void yaffs_bug_fn(const char *file_name, int line_no)
{
	printf("yaffs bug detected %s:%d\n",
		file_name, line_no);
	assert(0);
}
