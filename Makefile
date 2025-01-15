CC := $(CROSS_COMPILE)gcc
STRIP := $(CROSS_COMPILE)strip
CFLAGS := -Wall -Wextra -Werror -O2

ifeq ($(STATIC),1)
	CFLAGS += -static
endif

.PHONY: all clean

all: makerw

makerw: makerw.c
	$(CC) $(CFLAGS) -o $@ $<
	$(STRIP) $@

clean:
	rm -f makerw
	