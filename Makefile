CC := $(CROSS_COMPILE)gcc
CFLAGS := -Wall -Wextra -Werror -O2

ifeq ($(STATIC),1)
	CFLAGS += -static
endif

.PHONY: all clean

all: makerw

makerw: makerw.c
	$(CC) $(CFLAGS) -o $@ $<
	strip $@

clean:
	rm -f makerw