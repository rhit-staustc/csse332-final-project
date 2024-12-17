
all: commander.bin

CFLAGS += -ggdb -Wall -Werror

commander.bin: commander.o problem2.o helpers.o 
	${CC} -ggdb -Werror -Wall -o $@ $^ -lreadline

helpers.o: helpers.h

clean:
	rm -rf *.bin *.o
