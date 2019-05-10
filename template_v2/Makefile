file = src/main.c
cracker:$(file)
	gcc -Wall -Werror -lpthread -o cracker.o main.c reverse.c sha256.c
clean:
	rm -rf *.o *.bin
test:
	make cracker
	./cracker.o
	./cracker.o tests/4hash.bin
	./cracker.o tests/01_4c_1k.bin
	./cracker.o -t 6 tests/01_4c_1k.bin
	./cracker.o -c tests/01_4c_1k.bin
	./cracker.o -t 4 -c tests/01_4c_1k.bin tests/02_6c_5.bin
all:
	make cracker
	make test
