#!/usr/bin/env python3
"""
 * YAFFS: Yet another FFS. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2021 Aleph One Ltd.
 *
 * Created by Timothy Manning <timothy@yaffs.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

if __name__ == "__main__":
	#run the test runner.
	failed_tests = run(test_list)
	
	print("\ntest summary #############")
	if len(failed_tests) == 0:
		print('\033[42m' +"all tests passed"+'\033[0m')
	else:
		for path, output in failed_tests:
			print('\033[41m' +"test {} failed".format(path)+'\033[0m')
		print('\033[41m' + "ran {} tests, {} failed".format(len(test_list), len(failed_tests))+'\033[0m')
