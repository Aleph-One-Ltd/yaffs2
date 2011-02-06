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

#include "test_yaffs_unmount_ENODEV.h"

static int handle = 0;

int test_yaffs_unmount_ENODEV(void)
{
	int output=0;
	int error_code=0;

	handle = yaffs_unmount("/nonexisting_mount_point/");
	if (handle==-1){
		error_code = yaffs_get_error();
		if (abs(error_code) == ENODEV){
			return 1;
		} else {
			print_message("different error than expected\n",2);
			return -1;
		}
	} else if (output >=0){
		print_message("non existant directory opened.(which is a bad thing)\n",2);
		return -1;
	}
}

int test_yaffs_unmount_ENODEV_clean(void)
{
	return 1;
}

