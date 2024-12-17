all: problem3.bin

.PHONY: clean all

CFLAGS+= -ggdb -Wall -Werror

problem3.bin: problem3.o mystery.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf problem3.bin problem3.o

