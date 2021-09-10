#ifndef __SHARED_H__
#define __SHARED_H__

#define TEST_PASS 0
#define TEST_FAIL 1

#define YAFFS_MOUNT_POINT "/yflash2/"
#define FILE_PATH "/yflash2/foo.txt"

void setup_yaffs();
int shared_create();
int shared_validate_file();
#endif
