#==============================================================================
#
# objs
#
#==============================================================================
SRC = mcu-utils.c 

AP_NAME = mcu-utils

CFLAGS = -g -Wall -std=c++0x -fpermissive

LDFLAGS = -lpthread -lm 

all:
	make clean
	${CXX} -o $(AP_NAME) $(CFLAGS) $(LDFLAGS) ${SRC} -lpthread
	$(STRIP) --strip-debug $(AP_NAME)

clean:
	rm -f *.o *.o.elf $(AP_NAME)

%.o:%.c
	$(CXX) $(CFLAGS) -c -o $@ $<
	$(READELF) -a $@ > $@.elf

%.o:%.S
	$(CXX) $(CFLAGS) $(ASFLAGS) -c -o $@ $<
	$(READELF) -a $@ > $@.elf
