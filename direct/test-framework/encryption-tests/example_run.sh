#!/bin/bash
#
# Example runs for the encryption tester.
#
# Uses bash for lists.

# Rebuild
make clean
make || (echo "make failed... aborting"; exit 1)

# Run for a few combinations

page_sizes=("512" "2048")
tag_types=("inband" "oob")
encryptor_ids=("mbedtls" "unencrypted")

for page_size in "${page_sizes[@]}"; do
  for tag_type in "${tag_types[@]}"; do
    for encryptor_id in "${encryptor_ids[@]}"; do
    	echo "***"
    	echo "*** ./encryptor_test -d $page_size -t $tag_type -e $encryptor_id -v "
    	echo "***"
    	./encryptor_test -d $page_size -t $tag_type -e $encryptor_id -v
    	if [ $? != "0" ] ; then
    		echo "Bad test run... aborting"
    		exit 1
    	fi
    done
  done
done


echo "All test examples ran OK"
exit 0

