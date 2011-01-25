
How to initilise the nandsim

$ make

$ sudo -s
...password..
# now have a root shell
$ ./linux-tests/initnandsim 128MiB-2048
$ insmod yaffs2multi.ko
$ mkdir /mnt/y
$ mount -t yaffs2 /dev/mtdblock0 /mnt/y

How to change the permissions on the nandsim partition
$ sudo chmod a+wr /mnt/y/
#check the permission change
$ touch /mnt/y/test_file

How to clean the folder
$ rm -rf /mnt/y

The test must me run in sudo to work
