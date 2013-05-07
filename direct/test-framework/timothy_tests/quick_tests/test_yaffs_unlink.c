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

#include "test_yaffs_unlink.h"

int test_yaffs_unlink(void)
{
	if (yaffs_close(yaffs_open(FILE_PATH,O_CREAT | O_RDWR, FILE_MODE))==-1){
		print_message("failed to create file before remounting\n",1);
		return -1;
	}
	int output=yaffs_unlink(FILE_PATH);
	if (output>=0){
		return (-test_yaffs_access());	/*return negative access. we do not want the file to be there*/
	} else {
		print_message("failed to unlink file\n",2) ;
		return -1;
	}
}

int test_yaffs_unlink_clean(void)
{
	return test_yaffs_open()||test_yaffs_open_clean();
}
