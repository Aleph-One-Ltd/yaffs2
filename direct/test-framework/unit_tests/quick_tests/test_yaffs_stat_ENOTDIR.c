/*
 * YAFFS: Yet another FFS. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2018 Aleph One Ltd.
 *
 * Created by Timothy Manning <timothy@yaffs.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "test_yaffs_stat_ENOTDIR.h"

int test_yaffs_stat_ENOTDIR(void)
{
	int error_code=0;
	struct yaffs_stat stat;
	int output=0;

	if (yaffs_close(yaffs_open(FILE_PATH,O_CREAT | O_RDWR, FILE_MODE))==-1){
		print_message("failed to create file\n",1);
		return -1;
	}
	output=yaffs_stat(YAFFS_MOUNT_POINT "/test_dir/foo/file", &stat);;
	if (output<0){ 
		error_code=yaffs_get_error();
		if (abs(error_code)==ENOTDIR){
			return 1;
		} else {
			print_message("returned error does not match the the expected error\n",2);
			return -1;
		}
	} else {
		print_message("stated a non-existing file (which is a bad thing)\n",2);
		return -1;
	}	
}

int test_yaffs_stat_ENOTDIR_clean(void)
{
	return 1;
}
