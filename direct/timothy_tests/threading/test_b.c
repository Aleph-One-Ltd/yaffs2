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

#include "test_b.h"

void test_b(void *x)
{
	struct bovver_context *bc = (struct bovver_context *)x;
	int n = rand() % 20;
	
	bc->cycle++;

	if(!bc->dirH)
		bc->dirH = yaffs_opendir(bc->baseDir);

	if(!bc->dirH)
		return;

	if(n == 0){
		yaffs_closedir(bc->dirH);
		bc->dirH = NULL;
	} else {
		while(n > 1){
			n--;
			yaffs_readdir(bc->dirH);
		}
	}
}

