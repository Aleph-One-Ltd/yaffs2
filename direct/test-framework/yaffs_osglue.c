/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#include "yaffscfg.h"
#include "yaffs_guts.h"
#include "yaffsfs.h"
#include "yaffs_trace.h"
#include <assert.h>

#include <errno.h>


static int yaffsfs_lastError;

void yaffsfs_SetError(int err)
{
	//Do whatever to set error
	yaffsfs_lastError = err;
}

int yaffsfs_GetLastError(void)
{
	return yaffsfs_lastError;
}

int yaffsfs_CheckMemRegion(const void *addr, size_t size, int writeable)
{
	if(!addr)
		return -1;
	return 0;
}


#ifdef CONFIG_YAFFS_USE_PTHREADS
#include <pthread.h>
static pthread_mutex_t mutex1;


void yaffsfs_Lock(void)
{
	pthread_mutex_lock( &mutex1 );
}

void yaffsfs_Unlock(void)
{
	pthread_mutex_unlock( &mutex1 );
}

void yaffsfs_LockInit(void)
{
	pthread_mutex_init( &mutex1, NULL);
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

u32 yaffsfs_CurrentTime(void)
{
	return time(NULL);
}


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
	return this;
}

void yaffsfs_free(void *ptr)
{
	free(ptr);
}

void yaffsfs_OSInitialisation(void)
{
	yaffsfs_LockInit();
}


void yaffs_bug_fn(const char *file_name, int line_no)
{
	printf("yaffs bug detected %s:%d\n",
		file_name, line_no);
	assert(0);
}
