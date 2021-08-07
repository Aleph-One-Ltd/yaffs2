#!/usr/bin/env python3
"""
test_runner.py
This file runs all of the yaffs unit tests and aggrates the outputs.

To run this file you can use:
 
$ python3 test_runner.py

or 

$ ./test_runner.py

To add a new test to this test runner, add the test dir path to the
test_list below. Inside that folder there need to be a makefile with
a "test" target that compiles and runs the test script. I.e. "make test" 
is called in every test directory.
"""


import subprocess, sys

test_list = ["is_yaffs_working_tests",
	"quick_tests",
	"64_and_32_bit_time/64_bit",
	"64_and_32_bit_time/32_bit",
	]


TEST_FAILED = -1
TEST_PASSED = 1

def run(makefile_paths):
	failed_tests =[]
	for path in makefile_paths:
		print("\nrunning test {}".format(path))
		is_successful, test_output = run_makefile_test(path)
		if is_successful != TEST_PASSED:
			print('\033[41m' +'test {} failed'.format(path)+'\033[0m')
			print(test_output)
			failed_tests.append( (path, is_successful) )
		else:
			print('\033[42m' +"test passed"+'\033[0m')
	return failed_tests

def run_makefile_test(path):
	try:
		subprocess.check_output("make -j -C {} test".format(path), shell=True)	
	except subprocess.CalledProcessError as e:
		return (TEST_FAILED, e.output.decode('UTF-8'))
	
	return (TEST_PASSED, "test passed")

def clean_tests(makefile_paths):
    cmds = [("make -j -C {} clean".format(path), path) for path in makefile_paths]

    failed = 0
    passed = 0

    for cmd, is_successful, cmd_text, debug_info in run_cmds(cmds):
        if not is_successful:
            print("\033[41mtest failed to clean\033[0m {}".format(debug_info[0]))
            failed += 1
        else :
            print("\033[42mtest cleaned successfully\033[0m {}".format(debug_info[0]))
            passed += 1
    if not failed:
        print ("\n\033[42mAll tests cleaned successfully\033[0m")
    else :
        print ("\n\033[42mTests failed to clean successfully\033[0m")
    print("ran {}, passed {}, failed {}".format(len(cmds), passed, failed))

def run_tests(makefile_paths):
    cmds = [("make -j -C {} test".format(path), path) for path in makefile_paths]
    
    failed = 0
    passed = 0

    print("running tests")
    for cmd, is_successful, cmd_text, debug_info in run_cmds(cmds):
        if not is_successful:
            print("\033[41mtest failed\033[0m {}".format(debug_info[0]))
            failed += 1
        else :
            print("\033[42mtest passed\033[0m {}".format(debug_info[0]))
            passed += 1
    if not failed:
        print ("\n\033[42mAll tests passed\033[0m")
    else :
        print ("\n\033[41mTests failed\033[0m")
    print("ran {}, passed {}, failed {}".format(len(cmds), passed, failed)) 

def run_cmds(cmds):
    output = []
    for cmd, *debug_info in cmds:
        try:
            subprocess.check_output(cmd, shell=True)
            output.append((cmd, True, "todo add getting text for non failing test", debug_info))
        except subprocess.CalledProcessError as e:
            output.append((cmd, False, e.output.decode('UTF-8'), debug_info))
    return output
if __name__ == "__main__":
        if len(sys.argv) == 2 and sys.argv[1] == "clean":
            clean_tests(test_list)
        elif len(sys.argv) == 1:
            #run the test runner.
            failed_tests = run_tests(test_list)
        else:
            print("run with command ./test_runner.py [clean]")
