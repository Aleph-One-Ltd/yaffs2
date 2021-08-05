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

#include "test_yaffs_mkdir_EEXIST.h"

static int output = -1;

int test_yaffs_mkdir_EEXIST(void)
{
	int error_code = 0;
	
	output = yaffs_mkdir(DIR_PATH,O_CREAT | O_RDWR);
	if (output <0) {
		print_message("failed to create the directory the first time\n",2);
		return -1;
	}
	output = yaffs_mkdir(YAFFS_MOUNT_POINT "/test_dir/new_directory/",O_CREAT | O_RDWR);
	if (output < 0){
		error_code = yaffs_get_error();
		if (abs(error_code) == EEXIST){
			return 1;
		} else {
			print_message("different error than expected\n", 2);
			return -1;
		}
	} else {
		print_message("lseeked to a negative position (which is a bad thing)\n", 2);
		return -1;
	}
}


int test_yaffs_mkdir_EEXIST_clean(void)
{
	return yaffs_rmdir(DIR_PATH);
}

