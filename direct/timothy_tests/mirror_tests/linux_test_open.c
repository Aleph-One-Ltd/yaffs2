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

#include "linux_test_open.h"

int linux_test_open(arg_temp *args_struct)
{
	char path[250];
	char message[150];
	int output;
	join_paths(linux_struct.root_path,args_struct->string1, path );
	sprintf(message,"file path: %s\n",path);	
	print_message(3,message);

	output= open(path,args_struct->char1 & (O_TRUNC|O_EXCL|O_CREAT|O_APPEND) ,args_struct->char2& (S_IREAD|S_IWRITE));
	if (output>=0){
		output=close(output);
		if (output<0) {
			print_message(3,"failed to close handle\n");
			return -1;
		} else {
			return 1;
		}
	} else {
		print_message(3,"failed to open file\n");
		return -1;
	}
}
