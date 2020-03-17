#
#  Makefile for rtems-fsx.exe
#

#
#  RTEMS_MAKEFILE_PATH is typically set in an environment variable
#

EXEC=rtems-fsx.exe
PGM=${ARCH}/$(EXEC)

# optional managers required
MANAGERS=all

# C source names
CSRCS = rtems-fsx.c
CSRCS += yaffs-rtems-test-wrapper.c yaffs-rtems-flashsim.c
COBJS_ = $(CSRCS:.c=.o)
COBJS = $(COBJS_:%=${ARCH}/%)

# C++ source names
CXXSRCS =
CXXOBJS_ = $(CXXSRCS:.cc=.o)
CXXOBJS = $(CXXOBJS_:%=${ARCH}/%)

# AS source names
ASSRCS =
ASOBJS_ = $(ASSRCS:.s=.o)
ASOBJS = $(ASOBJS_:%=${ARCH}/%)

# Libraries
#LIBS = -lrtemsall -lc  -lyaffs2
LINK_LIBS = -lyaffs2

include $(RTEMS_MAKEFILE_PATH)/Makefile.inc

include $(RTEMS_CUSTOM)
include $(PROJECT_ROOT)/make/leaf.cfg

OBJS= $(COBJS) $(CXXOBJS) $(ASOBJS)

all:    ${ARCH} $(PGM)

#Create symlinks
yaffs-rtems-test-wrapper.c: ../common/yaffs-rtems-test-wrapper.c
	ln -sf $^ $@

yaffs-rtems-flashsim.c: ../common/yaffs-rtems-flashsim.c
	ln -sf $^ $@

yaffs-rtems-flashsim.h: ../common/yaffs-rtems-flashsim.h
	ln -sf $^ $@

$(OBJS): yaffs-rtems-flashsim.h

$(PGM): $(OBJS)
	$(make-exe)

