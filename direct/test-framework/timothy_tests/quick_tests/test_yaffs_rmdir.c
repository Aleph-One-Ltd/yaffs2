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

#include "test_yaffs_rmdir.h"


int test_yaffs_rmdir(void)
{
	int output=0;
	if (0 !=  yaffs_access(RMDIR_PATH,0)) {
		output = yaffs_mkdir(RMDIR_PATH,S_IWRITE | S_IREAD);
		if (output < 0) {
			print_message("failed to create directory\n",2);
			return -1;
		}
	}
	return yaffs_rmdir(RMDIR_PATH);
}


int test_yaffs_rmdir_clean(void)
{
	return 1;
}

