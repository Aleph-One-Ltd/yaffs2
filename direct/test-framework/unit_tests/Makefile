
TEST_DIR = quick_tests \
	is_yaffs_working_tests
	
all:
	$(foreach var,$(TEST_DIR), $(MAKE) -j -C $(var);)

phony. test:
	$(foreach var,$(TEST_DIR), $(MAKE) -j -C $(var) test;)
	
phony. clean:
	$(foreach var,$(TEST_DIR), $(MAKE) -j -C $(var) clean;)