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

#include "test_yaffs_sync_NULL.h"


int test_yaffs_sync_NULL(void)
{
	int error_code=-1;
	int output = yaffs_sync(NULL);
	
	if (output<0){
		error_code=yaffs_get_error();
		if (abs(error_code)==EFAULT){
			return 1;
		} else {
			print_message("returned error does not match the the expected error\n",2);
			return -1;
		}
	} else {
		print_message("synced a NULL path (which is a bad thing)\n",2);
		return -1;
	}	


}


int test_yaffs_sync_NULL_clean(void)
{
	return 1;
}

