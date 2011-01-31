/*
 * YAFFS: Yet another Flash File System . A NAND-flash specific file system. 
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Timothy Manning <timothy@yaffs.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * Note: Only YAFFS headers are LGPL, YAFFS C code is covered by GPL.
 */

#ifndef __thread_function_h__
#define __thread_function_h__


#include <string.h>
#include <stdlib.h>

#include "test_a.h"
#include "test_b.h"
#include "lib.h"
int thread_function(void * thread_id);
void * init(int threadId, const char *baseDir, int bovver_type);
#endif
