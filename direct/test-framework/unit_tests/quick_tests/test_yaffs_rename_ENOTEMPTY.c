/*
 * YAFFS: Yet another Flash File System . A NAND-flash specific file system. 
 *
 * Copyright (C) 2002-2018 Aleph One Ltd.
 *
 * Created by Timothy Manning <timothy@yaffs.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1 as
 * published by the Free Software Foundation.
 *
 * Note: Only YAFFS headers are LGPL, YAFFS C code is covered by GPL.
 */


#include "test_yaffs_rename_ENOTEMPTY.h"

int  test_yaffs_rename_ENOTEMPTY(void)
{

	int output=0;
	int error_code =0;

	if (yaffs_mkdir(DIR_PATH,O_CREAT | O_RDWR)==-1){
		print_message("failed to create dir\n",1);
		return -1;
	}
	if (yaffs_mkdir(DIR_PATH2,O_CREAT | O_RDWR)==-1){
		print_message("failed to create dir2\n",1);
		return -1;
	}
	if (yaffs_close(yaffs_open(DIR_PATH2_FILE,O_CREAT | O_RDWR, FILE_MODE))==-1){
		print_message("failed to create file\n",1);
		return -1;
	}
	output = yaffs_rename( DIR_PATH,DIR_PATH2 );
	if (output<0){ 
		error_code=yaffs_get_error();
		if (abs(error_code)==ENOTEMPTY){
			return 1;
		} else {
			print_message("different error than expected\n",2);
			return -1;
		}
		print_message("could not rename a directory over a nonempty directory (which is a bad thing)\n",2);
		return -1;
	} 
		
	return 1;

}


int  test_yaffs_rename_ENOTEMPTY_clean(void)
{
	return 1;
}

