#include "yaffsfs.h"
#include "yaffs_endian.h"

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

    struct yaffs_obj obj;
    if (sizeof(obj.yst_atime) != 8) {
	    printf("Error: size of yaffs_obj.yst_atime is not 64 bits\n");
	    return 1;
    }

    //create several times and save them
    //
    struct yaffs_dev dev;

    //need to set this to get the function to swap 
    //endianess of the values.

    //remember that the first two bytes are swapped to the 
    //other end while still retaining their order.
    // i.e. 0xfa.....  becomes 0x...fa
    dev.swap_endian = 1; 

    struct yaffs_obj_hdr oh;

    oh.yst_atime = 0xaf1732b0;
    oh.yst_mtime = 0xaf1732b0;
    oh.yst_ctime = 0xaf1732b0;

    yaffs_do_endian_oh(&dev,&oh);

    //check that the endianess is correct
    u32 expected = 0xb03217af;
    if (oh.yst_atime != expected ) {
        printf("endian test failed for yst_atime, got %x expected %x\n",oh.yst_atime,expected);
        return 1;
    } 
    if (oh.yst_mtime != expected) {
        printf("endian test failed for yst_mtime, got %x, expected %x\n",oh.yst_mtime,expected);
        return 1;
    }
    if (oh.yst_ctime != expected) {
        printf("endian test failed for yst_ctime, got %x, expected %x\n",oh.yst_ctime,expected);
        return 1;
    }

    printf("all tests pass\n");
    return 0;
}
