CFLAGS += -Wall -D_FILE_OFFSET_BITS=64

TARGETS = xattrtest writebigsparse verifybigsparse
all: $(TARGETS)

xattrtest: xattrtest.c
	gcc $(CFLAGS) -o $@ $<

writebigsparse: writebigsparse.c
	gcc $(CFLAGS) -o $@ $<

verifybigsparse: verifybigsparse.c
	gcc $(CFLAGS) -o $@ $<
	

.PHONEY: clean
clean:
	rm -f $(TARGETS)
