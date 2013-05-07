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

#include "test_a.h"





void test_a(void *x)
{
	struct bovver_context *bc = (struct bovver_context *)x;

	int i;
	int op;
	int pos;
	int n;
	int n1;
	struct yaffs_stat stat_buffer;
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
	op-=1;
	if(op < 0){
		yaffs_rmdir(name);
		return;
	}
	op -=1;
	if(op < 0){
		if(bc->h[i]>= 0){
			yaffs_fsync(bc->h[i]);
			return;
		}
	}
	op -=1;
	if(op < 0){
		if(bc->h[i]>= 0){
			yaffs_fdatasync(bc->h[i]);
			return;
		}
	}
	op -=1;
	if(op < 0){
		if(bc->h[i]>= 0){
			yaffs_flush(bc->h[i]);
			return;
		}
	}

	op -=1;
	if(op < 0){
		if((bc->h[i]>= 0) && (bc->h[i+1] < 0)){
			bc->h[i+1]=yaffs_dup(bc->h[i]);
			return;
		}
	}
	op -=1;
	if(op < 0){
		if(bc->h[i]>= 0){
			yaffs_ftruncate(bc->h[i],n);
			return;
		}
	}

	op -=1;
	if(op < 0){
		if(bc->h[i]>= 0){
			yaffs_fstat(bc->h[i],&stat_buffer);
			yaffs_fchmod(bc->h[i], n);
			yaffs_fchmod(bc->h[i], ((S_IREAD|S_IWRITE)&(stat_buffer.st_mode)));
		}
	}
	op -=1;
	if(op < 0){
		if(bc->h[i]>= 0){
			yaffs_stat(name,&stat_buffer);
			yaffs_chmod(name, n);
			yaffs_chmod(name, ((S_IREAD|S_IWRITE)&(stat_buffer.st_mode)));
		}
	}
	op -=1;
	if(op < 0){
		if(bc->h[i]>= 0){
			yaffs_lstat(name,&stat_buffer);
		}
	}
	op -=1;
	if(op < 0){
		if(bc->h[i]>= 0){
			yaffs_read(bc->h[i],name,n);
			return;
		}
	}
	op -=1;
	if(op < 0){
		if(bc->h[i]>= 0){
			yaffs_pread(bc->h[i],name,n,n1);
			return;
		}
	}
	op -=1;
	if(op < 0){
		if(bc->h[i]>= 0){
			yaffs_write(bc->h[i],name,n);
			return;
		}
	}
	op -=1;
	if(op < 0){
		if(bc->h[i]>= 0){
			yaffs_pwrite(bc->h[i],name,n,n1);
			return;
		}
	}
	op -=1;
	if(op < 0){
		if(bc->h[i]>= 0){
			yaffs_truncate(name,n);
			return;
		}
	}
	op -=1;
	if(op < 0){
		if(bc->h[i]>= 0){
			yaffs_access(name,n);
			return;
		}
	}
	op -=1;
	if(op < 0){
		if(bc->h[i]>= 0){
			yaffs_symlink(name,name1);
			return;
		}
	}
	op -=1;
	if(op < 0){
		if(bc->h[i]>= 0){
			yaffs_link(name,name1);
			return;
		}
	}
		op -=1;
	if(op < 0){
		if(bc->h[i]>= 0){
			yaffs_unlink(name);
			return;
		}
	}
	return;		
	
}

