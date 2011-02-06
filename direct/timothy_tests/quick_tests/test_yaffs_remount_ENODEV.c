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

#include "test_yaffs_remount_ENODEV.h"

int test_yaffs_remount_ENODEV(void)
{
	int output = -1;
	int error_code =0;


	output = yaffs_remount("non-existing-mount-point",0,0);
	if (output<0){
		error_code = yaffs_get_error();
		if (abs(error_code) == ENODEV){
			return 1;
		} else {
			print_message("returned error does not match the the expected error\n",2);
			return -1;
		}
	} else {
		print_message("remounted a non-existing-dir\n",2);
		return -1;
	}
}

int test_yaffs_remount_ENODEV_clean(void)
{
	int output=0;
	int error_code =0;
	output= yaffs_mount(YAFFS_MOUNT_POINT);
	if (output<0){
		error_code=yaffs_get_error();
		if (abs(error_code) == EBUSY){
			return 1;
		} else {
			return -1;
		}
	}
	return 1;
}
