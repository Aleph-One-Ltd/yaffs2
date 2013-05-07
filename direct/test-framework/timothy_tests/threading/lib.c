/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Timothy Manning <timothy@yaffs.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "lib.h"
static volatile unsigned int *p_counter;
static unsigned int numberOfThreads=5;

unsigned int get_num_of_threads(void)
{
	return numberOfThreads;
}

void number_of_threads(unsigned int num)
{
	numberOfThreads=num;
}

int get_counter(int x)
{
	return p_counter[x];
}

void set_counter(int x, unsigned int value)
{
	p_counter[x]=value;
}

void init_counter(unsigned int size_of_counter)
{
	p_counter=malloc(size_of_counter*sizeof(unsigned int));
}

