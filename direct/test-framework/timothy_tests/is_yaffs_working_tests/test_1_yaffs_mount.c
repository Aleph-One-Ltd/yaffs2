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


#include "yaffsfs.h"
#define YAFFS_MOUNT_POINT "/yaffs2/"
#define FILE_PATH "/yaffs2/foo.txt"

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
		printf("yaffs mounted: %s\n",YAFFS_MOUNT_POINT); 
	} else {
		printf("error\n yaffs failed to mount: %s\nerror\n",YAFFS_MOUNT_POINT);
		return (0);
	}
	//now create a file.
	output = yaffs_open(FILE_PATH,O_CREAT | O_RDWR, S_IREAD | S_IWRITE);
	if (output>=0){  
		printf("file created: %s\n",FILE_PATH); 
	} else {
		printf("error\n yaffs failed to create the file: %s\nerror\n",FILE_PATH);
		return (0);
	}
	output2 = yaffs_close(output);
	if (output2>=0){  
		printf("file closed: %s\n",FILE_PATH); 
	} else {
		printf("error\n yaffs failed to close the file: %s\nerror\n",FILE_PATH);
		return (0);
	}
	//unmount and remount the mount point.
	output = yaffs_unmount(YAFFS_MOUNT_POINT);
	if (output>=0){  
		printf("yaffs unmounted: %s\n",YAFFS_MOUNT_POINT); 
	} else {
		printf("error\n yaffs failed to unmount: %s\nerror\n",YAFFS_MOUNT_POINT);
		return (0);
	}
	output = yaffs_mount(YAFFS_MOUNT_POINT);
	if (output>=0){  
		printf("yaffs mounted: %s\n",YAFFS_MOUNT_POINT); 
	} else {
		printf("error\n yaffs failed to mount: %s\nerror\n",YAFFS_MOUNT_POINT);
		return (0);
	}
	//now open the existing file.
	output = yaffs_open(FILE_PATH, O_RDWR, S_IREAD | S_IWRITE);
	if (output>=0){  
		printf("file created: %s\n",FILE_PATH); 
	} else {
		printf("error\n yaffs failed to create the file: %s\nerror\n",FILE_PATH);
		return (0);
	}
	//close the file.
	output2 = yaffs_close(output);
	if (output2>=0){  
		printf("file closed: %s\n",FILE_PATH); 
	} else {
		printf("error\n yaffs failed to close the file: %s\nerror\n",FILE_PATH);
		return (0);
	}

	//unmount the mount point.
	output = yaffs_unmount(YAFFS_MOUNT_POINT);
	if (output>=0){  
		printf("yaffs unmounted: %s\n",YAFFS_MOUNT_POINT); 
	} else {
		printf("error\n yaffs failed to unmount: %s\nerror\n",YAFFS_MOUNT_POINT);
		return (0);
	}

	printf("test passed. yay!\n");
	
}
