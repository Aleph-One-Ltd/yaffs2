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

#include "test_yaffs_rename_file_to_dir.h"


int test_yaffs_rename_file_to_dir(void)
{

	int output=0;
	int error_code=0;
	if (yaffs_close(yaffs_open(FILE_PATH,O_CREAT | O_RDWR, FILE_MODE))==-1){
		print_message("failed to create file\n",1);
		return -1;
	}

	if (0 !=  yaffs_access(RENAME_DIR_PATH,0)) {
		output = yaffs_mkdir(RENAME_DIR_PATH,S_IWRITE | S_IREAD);
		if (output < 0) {
			print_message("failed to create directory\n",2);
			return -1;
		}
	}
	output=yaffs_rename(FILE_PATH,RENAME_DIR_PATH);
	if (output==-1){
		error_code=yaffs_get_error();
		if (abs(error_code)==EISDIR){
			return 1;
		} else {
			print_message("different error than expected\n",2);
			return -1;
		}
	} else {
		print_message("renamed a file over a directory (which is a bad thing)\n",2);
		return -1;
	}
	return 1;

}


int test_yaffs_rename_file_to_dir_clean(void)
{
	
	return 1;

}

