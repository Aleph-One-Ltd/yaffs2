#include "yaffsfs.h"
#define YAFFS_MOUNT_POINT "/yflash2/"
#define FILE_PATH "/yflash2/foo.txt"

int random_seed;
int simulate_power_failure = 0;


int main()
{
	yaffs_start_up();

    //test that ytime is 64 bits

    if (sizeof(YTIME_T) != 8) {
        printf("Error:size of YTIME_T is not 64 bits\n");
        return 1;
    }


    //create several times and save them
    //

    //extra tests
    //save the time and get it to overflow.
    printf("all tests pass\n");
    return 0;
}
