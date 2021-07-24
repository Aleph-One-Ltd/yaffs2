#include "yaffsfs.h"
#define YAFFS_MOUNT_POINT "/yflash2/"
#define FILE_PATH "/yflash2/foo.txt"

int random_seed;
int simulate_power_failure = 0;


int main()
{
	yaffs_start_up();

    //test that ytime is 32 bits

    if (sizeof(YTIME_T) != 4) {
        printf("Error:size of YTIME_T is %lu, not 32 bits \n", sizeof(YTIME_T)*8);
        return 1;
    }


    //create several times and save them
    //

    //extra tests
    //save the time and get it to overflow.
    printf("all tests pass\n");
    return 0;
}
