
all: problem1.bin caesar.bin

CFLAGS += -ggdb -Wall -Werror

problem1.bin: problem1.o helpers.o 
	${CC} -ggdb -Werror -Wall -o $@ $^ 

caesar.bin: caesar.o helpers.o 
	${CC} -ggdb -Werror -Wall -o $@ $^

helpers.o: helpers.h

clean:
	rm -rf *.bin *.o
