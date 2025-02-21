
TARGET := lte-apn

APL_DIR := .

SOURCES	:= $(APL_DIR)/xmlparser.cpp \
		$(APL_DIR)/at_tok.cpp \
		$(APL_DIR)/lte_api.cpp \
		$(APL_DIR)/lte-apn.cpp \

OBJS := $(patsubst %.cpp,%.o,$(SOURCES))

CFLAGS += -Wall -I$(APL_DIR)
#CFLAGS += -D_DEBUG

.PHONY: all clean

all:    $(TARGET)
$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) -lxml2

%.o: %.cpp
	$(CC) $(CFLAGS) -c $<  -o $@

clean:
	$(RM) $(TARGET)
	$(RM) $(APL_DIR)/*.o
