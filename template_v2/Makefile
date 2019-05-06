file = src/main.c
cracker:$(file)
	gcc -Wall -Werror -o cracker.o src/*.c
clean:
	rm -rf *.o *.bin
test:
	./cracker.o -t 4 ../test-input/01_4c_1k.bin
all:
	make cracker 
	make test
	