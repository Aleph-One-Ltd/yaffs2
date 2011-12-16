
linux_test.c tests yaffs running under linux using the nandsim generator.  


If a segmentation fault happens during the test then check that 
the nandsim has been initilised properly. 

How to initilise the nandsim

$ make

$ sudo -s
...password..
# now you have a root shell
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


The test must be run in sudo to work to allow the files to be 
created in the root folders.

compile command: make
run command: sudo ./linux_test

command line options:
	-h			display the command line options.
	-s [number]	seeds the rand with the number.
	-p [number]	sets the print level to the number. 
				the higher the number the more low level commands are printed.
				the number should be between 0 and 5.
	-v 			verbose mode. everything is printed.
	-q			quite mode. nothing is printed.

 
