#include <stdlib.h>
#include "yaffsfs.h"
#include "shared.h"

void assert_exit_yaffs(char *fun_name, int fun_return_value){
	if (fun_return_value < 0) {
		printf("yaffs command: %s failed with error code %d \n", fun_name, fun_return_value);
		int error_code = yaffsfs_GetLastError();
		printf("error code is: %d, which is %s\n", error_code, yaffs_error_to_str(error_code));
		printf("exiting program now\n");
		exit(-1);
	}
}

void setup_yaffs() {
	yaffs_start_up();
	yaffs_set_trace(0);
	assert_exit_yaffs("yaffs_mount", yaffs_mount(YAFFS_MOUNT_POINT));
}



int shared_create(int argc, char *argv[]){
	printf("YTIME_T size is %lu bits\n", sizeof(YTIME_T) *8);
	#ifdef CONFIG_YAFFS_USE_32_BIT_TIME_T
		if (sizeof(YTIME_T)*8 != 32) {
			printf("YTIME_T size is incorrect. it is %lu and should be 32\n", sizeof(YTIME_T));
		} 
	#endif
	if (argc != 3) {
		printf("wrong number of arguments\n");
		printf("requires $ create file_name time\n");
		return TEST_FAIL;
	}
	
	setup_yaffs();	
	char *file_path = argv[1];
	int handle = yaffs_open(file_path, O_CREAT | O_RDWR, S_IREAD |S_IWRITE);
	assert_exit_yaffs ( "yaffs_open", handle);

	assert_exit_yaffs ( "yaffs_close", yaffs_close(handle));
	
	
	//set the time.
	uint time = atoi(argv[2]);

	struct yaffs_utimbuf new_times;
	new_times.actime = time;
	int ret = yaffs_utime(file_path, &new_times);
	assert_exit_yaffs("yaffs_utime", ret);

	assert_exit_yaffs("yaffs_unmount", yaffs_unmount(YAFFS_MOUNT_POINT));
	return 0;
}

int shared_validate_file(int argc, char *argv[]){
	
	if (argc != 3) {
		printf("wrong number of arguments\n");
		printf("requires $ create file_name time\n");
		return TEST_FAIL;
	}
	
	setup_yaffs();	
	char *file_path = argv[1];
	int handle = yaffs_open(file_path, O_RDWR, S_IREAD |S_IWRITE);
	assert_exit_yaffs ( "yaffs_open", handle);

	//assert the file is the correct size and has the correct time.
	uint expected_time = atoi(argv[2]);	
	struct yaffs_stat stat;
	assert_exit_yaffs ( "yaffs_stat", yaffs_stat(file_path, &stat));
	assert_exit_yaffs ( "yaffs_close", yaffs_close(handle));
	if (stat.yst_atime != expected_time) {
		printf("stat time is different from expected time\n");
		exit(0);
	}
	assert_exit_yaffs("yaffs_unmount", yaffs_unmount(YAFFS_MOUNT_POINT));
	return 0;
}
