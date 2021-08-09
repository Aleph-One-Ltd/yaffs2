//#include <stdio.h>
#include <stdlib.h>
#include "yaffsfs.h"

#define TEST_PASS 0
#define TEST_FAIL 1

#define YAFFS_MOUNT_POINT "/yflash2/"
#define FILE_PATH "/yflash2/foo.txt"

int random_seed;
int simulate_power_failure = 0;


int setup_yaffs() {
	yaffs_start_up();
	yaffs_set_trace(0);
	if (yaffs_mount(YAFFS_MOUNT_POINT) < 0) {
		printf("failed to mount %s/n", YAFFS_MOUNT_POINT);
		return TEST_FAIL;
	}	
	
	return TEST_PASS;
}
int shared_create(int argc, char *argv[]){
	
	if (argc != 3) {
		printf("wrong number of arguments\n");
		printf("requires $ create file_name time\n");
		return TEST_FAIL;
	}
	
	setup_yaffs();	
	uint time = atoi(argv[2]);
	char *file_path = argv[1];
	if (yaffs_open(FILE_PATH, O_CREAT | O_RDWR, S_IREAD |S_IWRITE)) {
		printf("failed to open the file %s/n", FILE_PATH);
		return TEST_FAIL;
	}
	printf("created file: %s,  with  time: %d\n", file_path, time);
	return TEST_PASS;
}
int main(int argc, char *argv[] ){
	return shared_create(argc, argv);
}
