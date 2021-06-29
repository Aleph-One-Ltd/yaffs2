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

#include "test_yaffs_open_EROFS.h"
#include "test_yaffs_open.h"

static int output =0;

int test_yaffs_unlink_EROFS(void)
{

	int error_code=0;
	if (yaffs_close(yaffs_open(FILE_PATH,O_CREAT | O_RDWR, FILE_MODE))==-1){
		print_message("failed to create file before remounting\n",1);
		return -1;
	}
	EROFS_setup();

	output=yaffs_unlink(FILE_PATH);
	if (output==-1){
		error_code=yaffs_get_error();
		if (abs(error_code)==EROFS){
			return 1;
		} else {
			print_message("different error than expected\n",2);
			return -1;
		}
	} else {
		print_message("unlinked a directory with EROFS set.(which is a bad thing)\n",2);
		return -1;
	}


}
int test_yaffs_unlink_EROFS_clean(void)
{	
	return EROFS_clean()&& yaffs_close(test_yaffs_open()) ;
}

