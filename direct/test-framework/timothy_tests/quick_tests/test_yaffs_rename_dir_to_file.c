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

#include "test_yaffs_rename_dir_to_file.h"

/*tries to rename a directory over an existing file */
int test_yaffs_rename_dir_to_file(void)
{
	int output=0;
	int error_code=0;
	if (0 !=  yaffs_access(DIR_PATH,0)) {
		output= yaffs_mkdir(DIR_PATH,(S_IREAD | S_IWRITE));
		if (output<0) {
			print_message("failed to create directory\n",2);
			return -1;
		}
	}
	if (yaffs_close(yaffs_open(FILE_PATH,O_CREAT | O_RDWR, FILE_MODE))==-1){
		print_message("failed to create file\n",1);
		return -1;
	}
	output = yaffs_rename( DIR_PATH , FILE_PATH);
	if (output==-1){
		error_code=yaffs_get_error();
		if (abs(error_code)==ENOTDIR){
			return 1;
		} else {
			print_message("different error than expected\n",2);
			return -1;
		}
	} else {
		print_message("renamed a directory over file.(which is a bad thing)\n",2);
		return -1;
	}

}


int test_yaffs_rename_dir_to_file_clean(void)
{
	int output = 0;
	if (0 ==  yaffs_access(RENAME_DIR_PATH,0)) {
		output = yaffs_rename(RENAME_DIR_PATH,DIR_PATH);
		if (output < 0) {
			print_message("failed to rename the file\n",2);
			return -1;
		}
	}
	return 1;

}

