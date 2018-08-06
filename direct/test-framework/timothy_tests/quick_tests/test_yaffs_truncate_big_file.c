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

#include "test_yaffs_truncate_big_file.h"


int test_yaffs_truncate_big_file(void)
{
	int error=0;
	int output=0;

	output= yaffs_truncate(YAFFS_MOUNT_POINT "/foo", YAFFS_TEST_LONG_VALUE);
	if (output<0){
		error=yaffs_get_error();
		if (abs(error)==EINVAL){	/*in yaffs EINVAL is used instead of big_file */
			return 1;
		} else {
			print_message("received a different error than expected\n",2);
			return -1;
		}
	} else{
		print_message("truncated a file to a massive size\n",2);
		return -1;
	}
			

}

int test_yaffs_truncate_big_file_clean(void)
{
	return 1;
}
