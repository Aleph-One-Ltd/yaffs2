Made by Timothy Manning on 24/12/2010 <timothy@yaffs.net>


	mirror_tests is designed to check that yaffs behaves the same way as linux.
	This is done by applying the same command to both linux and yaffs and 
	then checking the output of the functions.
	The default place for creating linux files is direct/timothy_tests/mirror_tests/tests/
	This directory is removed and is recreated at the beginning of each test, 
	However some of the files may be read only and cannot be deleted.
	It is much better to remove this directory via the command line.

	rm test/*;rmdir test/;./mirror_tests -n 100 -v

compile command: make
run command: mirror_tests

Command line arguments
	-yaffs_path [PATH] 		sets the path for yaffs.
	-linux_path [PATH] 		sets the path for linux.
	-p [NUMBER] 			sets the print level for mirror_tests.
	-v 						verbose mode everything is printed
	-q 						quiet mode nothing is printed.
	-n [number] 			sets the number of random tests to run.
	-s [number] 			seeds rand with the number
	-t [number] 			sets yaffs_trace to the number
	-clean 					removes emfile and test dir

