#!/bin/bash
# Run this to initialise the file system for the test runs.
set -x
rm seed-m18-*
rm emfile-m18*
./yaffs_test  -u -i M18-1
