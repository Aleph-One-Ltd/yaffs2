# Encryptor Testing README

The purpose of the encryptor tester is to provide a framework for doing some testing of
a Yaffs encryptor.

The code is currently built to test either unencrypted volumes (ie. to test the built in
null encryptor as well as the example encryptor using MbedTLS (yaffs_encrypt_mbedtls.c).

# Building the test harness.

First we need to copy the Yaffs core code into Yaffs direct:

```
cd yaffs2/direct
./handle_common.sh refresh
```

Next we go to the encryption test directory and build there

```
cd yaffs2/direct/test-framework/encryption-tests
make clean; make
```

# Running the tests

