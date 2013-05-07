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

#include "test_yaffs_truncate_EINVAL.h"


int test_yaffs_truncate_EINVAL(void)
{
	int error=0;
	int output=0;
	if (yaffs_close(yaffs_open(FILE_PATH,O_CREAT | O_RDWR, FILE_MODE))==-1){
		print_message("failed to create file \n",1);
		return -1;
	}
	output= yaffs_truncate(FILE_PATH,-1 );
	if (output<0){
		error=yaffs_get_error();
		if (abs(error)==EINVAL){
			return 1;
		} else {
			print_message("received a different error than expected\n",2);
			return -1;
		}
	} else{
		print_message("truncated a file with a bad mode set.\n",2);
		return -1;
	}
			

}

int test_yaffs_truncate_EINVAL_clean(void)
{
	return 1;
}
