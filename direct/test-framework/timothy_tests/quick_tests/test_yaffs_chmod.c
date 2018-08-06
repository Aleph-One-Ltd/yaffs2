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

#include "test_yaffs_chmod.h"

int test_yaffs_chmod(void)
{
	int output=0;
	if (yaffs_close(yaffs_open(FILE_PATH,O_CREAT | O_RDWR, FILE_MODE))==-1){
		print_message("failed to create file\n",1);
		return -1;
	}
	
	output=yaffs_chmod(FILE_PATH,S_IREAD|S_IWRITE);

	if (output>=0){
		/* there were no errors */
		return 1;
	} else {
		print_message("failed to chmod the file\n",2);
		return -1;
	}

}

int test_yaffs_chmod_clean(void)
{
	return 1;
}
