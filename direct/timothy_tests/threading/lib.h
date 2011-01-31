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

#ifndef __lib_h__
#define __lib_h__

#define BOVVER_HANDLES 10
#include "yaffsfs.h"
struct bovver_context {
	int bovverType;
	int threadId;
	char baseDir[200];
	int h[BOVVER_HANDLES];
	yaffs_DIR *dirH;
	int opMax;
	int op;
	int cycle;
};

int get_counter(int x);
void set_counter(int x, unsigned int value);
void init_counter(unsigned int size_of_counter);
void number_of_threads(unsigned int num);
unsigned int get_num_of_threads(void);

#endif
