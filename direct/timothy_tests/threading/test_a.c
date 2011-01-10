/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2010 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Timothy Manning <timothy@yaffs.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "test_a.h"





void test_a(void *x)
{
	struct bovver_context *bc = (struct bovver_context *)x;

	int i;
	int op;
	int pos;
	int n;
	int n1;
	
	char name[200];
	char name1[200];
	
	int start_op;
	
	
	i = rand() % BOVVER_HANDLES;
	op = rand() % bc->opMax;
	pos = rand() & 20000000;
	n = rand() % 100;
	n1 = rand() % 100;
	
	start_op = op;
		
	sprintf(name, "%s/xx%d",bc->baseDir,n);
	sprintf(name1,"%s/xx%d",bc->baseDir,n1);

	bc->op = op;
	bc->cycle++;
	
	op-=1;
	if(op < 0){
		if(bc->h[i]>= 0){
			yaffs_close(bc->h[i]);
			bc->h[i] = -1;
		}
		return;
	}

	op-=1;
	if(op < 0){
		if(bc->h[i] < 0)
			bc->h[i] = yaffs_open(name,O_CREAT| O_RDWR, 0666);
		return;
	}

	op-=5;
	if(op< 0){
		yaffs_lseek(bc->h[i],pos,SEEK_SET);
		yaffs_write(bc->h[i],name,n);
		return;
	}

	op-=1;
	if(op < 0){
		yaffs_unlink(name);
		return;
	}
	op-=1;
	if(op < 0){
		yaffs_rename(name,name1);
		return;
	}
	op-=1;
	if(op < 0){
		yaffs_mkdir(name,0666);
		return;
	}
	op-=1;
	if(op < 0){
		yaffs_rmdir(name);
		return;
	}

	bc->opMax = (start_op - op -1);
	
	return;		
	
}

