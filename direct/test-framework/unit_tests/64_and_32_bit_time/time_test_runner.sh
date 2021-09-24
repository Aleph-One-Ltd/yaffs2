make -C  32_bit/ && make -C 64_bit/ &&  
	rm emfile-* && 
	./32_bit/create_32_bit /yflash2/foo 100 && 
	echo created 32 bit file &&
	./64_bit/validate_64_bit /yflash2/foo 100 &&
	echo 32 bit file opened with 64 bit mode correctly

	rm emfile-* && 
	./64_bit/create_64_bit /yflash2/foo 100 && 
	echo created 64 bit file &&
	./32_bit/validate_32_bit /yflash2/foo 100 &&
	echo 64 bit file opened with 32 bit mode correctly &&

	echo All tests ran properly
