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

#include "test_yaffs_truncate_ENOTDIR.h"



int test_yaffs_truncate_ENOTDIR(void)
{
	int error=0;
	int output=0;
	if (yaffs_close(yaffs_open(FILE_PATH,O_CREAT | O_RDWR, FILE_MODE))==-1){
		print_message("failed to create file \n",1);
		return -1;
	}

	output= yaffs_truncate("/yaffs2/test_dir/foo/file",FILE_SIZE_TRUNCATED );
	if (output<0){
		error=yaffs_get_error();
		if (abs(error)==ENOTDIR){
			return 1;
		} else {
			print_message("received a different error than expected\n",2);
			return -1;
		}
	} else{
		print_message("truncated a nonexisting file\n",2);
		return -1;
	}
}

int test_yaffs_truncate_ENOTDIR_clean(void)
{
	return 1;
}
