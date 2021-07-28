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


#include "yaffsfs.h"
#define YAFFS_MOUNT_POINT "/yflash2/"
#define FILE_PATH "/yflash2/foo.txt"

int random_seed;
int simulate_power_failure = 0;


int main()
{
	int output = 0;
	int output2 = 0;
	yaffs_start_up();

	printf("\n\n starting test\n");
	yaffs_set_trace(0);
	output = yaffs_mount(YAFFS_MOUNT_POINT);

	if (output>=0){  
		printf("yaffs correctly mounted: %s\n",YAFFS_MOUNT_POINT);
	} else {
		printf("error\n yaffs failed to mount: %s\n with error code %d\n",YAFFS_MOUNT_POINT, yaffs_get_error());

		return (-1);
	}
	//now create a file.
	output = yaffs_open(FILE_PATH,O_CREAT | O_RDWR, S_IREAD | S_IWRITE);
	if (output>=0){  
		printf("yaffs correctly created a the file: %s\n",FILE_PATH);
	} else {
		printf("error\n yaffs failed to create the file: %s\nerror\n",FILE_PATH);
		return (-1);
	}
	output2 = yaffs_close(output);
	if (output2>=0){  
		printf("yaffs correctly closed the file: %s\n",FILE_PATH);
	} else {
		printf("error\n yaffs failed to close the file: %s\nerror\n",FILE_PATH);
		return (-1);
	}
	//unmount and remount the mount point.
	output = yaffs_unmount(YAFFS_MOUNT_POINT);
	if (output>=0){  
		printf("yaffs correctly unmounted: %s\n",YAFFS_MOUNT_POINT);
	} else {
		printf("error\n yaffs failed to unmount: %s\nerror\n",YAFFS_MOUNT_POINT);
		return (-1);
	}
	output = yaffs_mount(YAFFS_MOUNT_POINT);
	if (output>=0){  
		printf("yaffs correctly mounted: %s\n",YAFFS_MOUNT_POINT);
	} else {
		printf("error\n yaffs failed to mount: %s\nerror\n",YAFFS_MOUNT_POINT);
		return (-1);
	}
	//now open the existing file.
	output = yaffs_open(FILE_PATH, O_RDWR, S_IREAD | S_IWRITE);
	if (output>=0){  
		printf("yaffs correctly opened the file: %s\n",FILE_PATH);
	} else {
		printf("error\n yaffs failed to create the file: %s\nerror\n",FILE_PATH);
		return (-1);
	}
	//close the file.
	output2 = yaffs_close(output);
	if (output2>=0){  
		printf("yaffs correctly closed the file: %s\n",FILE_PATH);
	} else {
		printf("error\n yaffs failed to close the file: %s\nerror\n",FILE_PATH);
		return (-1);
	}

	//unmount the mount point.
	output = yaffs_unmount(YAFFS_MOUNT_POINT);
	if (output>=0){  
		printf("yaffs correctly unmounted: %s\n",YAFFS_MOUNT_POINT);
	} else {
		printf("error\n yaffs failed to unmount: %s\nerror\n",YAFFS_MOUNT_POINT);
		return (-1);
	}

	printf("test passed. yay!\n");
	
	return(0);
}
