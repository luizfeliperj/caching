CFLAGS=-g -I.
CC=/usr/bin/gcc
OBJS:=$(shell /bin/ls *.c | /bin/sed -e 's,\.c,\.o,' -e 's,.*,\.obj/\0,' )

.PHONY: all clean

all: caching.x

caching.x: .obj/caching.o
	$(CC) $(CFLAGS) -o $@ $^

.obj/%.o: %.c 
	/bin/mkdir -p .obj && $(CC) $(CFLAGS) $(STD_CFLAGS) -c -o $@ $<

clean:
	/bin/rm -rf .obj caching.x
