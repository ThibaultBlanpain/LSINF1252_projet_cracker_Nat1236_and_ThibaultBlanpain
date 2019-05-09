file = src/main.c
cracker:$(file)
	gcc -Wall -Werror -o cracker.o src/*.c
clean:
	rm -rf *.o *.bin
test:
	make cracker
	./cracker.o
	./cracker.o 4hash.bin
	./cracker.o 01_4c_1k.bin
	./cracker.o -t 6 01_4c_1k.bin
	./cracker.o -c 01_4c_1k.bin
	./cracker.o -t 4 -c ../test-input/01_4c_1k.bin ../test-input/02_6c_5.bin
all:
	make cracker 
	make test
	