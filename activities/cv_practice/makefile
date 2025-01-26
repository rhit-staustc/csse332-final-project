SOURCES:=$(wildcard *.c)
TARGETS:=$(SOURCES:%.c=%.bin)

all: ${TARGETS}

CFLAGS+=-ggdb -Wall
LDFLAGS+=-pthread

.PHONY: all clean

%.bin: %.o
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^

clean:
	rm -f *.o *.bin
