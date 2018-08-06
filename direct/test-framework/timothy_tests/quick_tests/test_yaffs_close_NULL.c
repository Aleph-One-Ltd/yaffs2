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

/* generates a EBADF error by closing a daft handle*/

#include "test_yaffs_close_NULL.h"

static int handle = -1;

int test_yaffs_close_NULL(void)
{
	int output = 0;
	int error_code = 0;


	output = yaffs_close(999);
	if (output < 0){
		error_code = yaffs_get_error();
		if (abs(error_code) == EBADF){
			handle =-1;
			return 1;
		} else {
			print_message("different error than expected\n",2);
			return -1;
		}
	} else {
		print_message("closed a NULL handle. (which is a bad thing)\n", 2);
		return -1;
	}

}


int test_yaffs_close_NULL_clean(void)
{
	if (handle >= 0){
		printf("handle %d\n",handle);
		return yaffs_close(handle);
	} else {
		return 1;
	}
}

