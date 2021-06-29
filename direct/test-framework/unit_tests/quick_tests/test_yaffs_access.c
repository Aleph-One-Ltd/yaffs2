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

#include "test_yaffs_access.h"

int test_yaffs_access(void)
{

	if (-1==yaffs_close(yaffs_open(FILE_PATH,O_CREAT | O_RDWR, FILE_MODE))){
		print_message("failed to create file\n",1);
		return -1;
	}
	return yaffs_access(FILE_PATH,0);
}

int test_yaffs_access_clean(void)
{
	return 1;
}
