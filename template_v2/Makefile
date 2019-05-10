file = src/main.c src/reverse.c src/sha256.c src/reverse.h src/sha256.h
cracker:$(file)
	gcc -Wall -Werror -o cracker src/main.c src/reverse.c src/sha256.c -lpthread
tests:
	make cracker
	./cracker tests/4hash.bin
	./cracker tests/01_4c_1k.bin
	./cracker -t 6 tests/01_4c_1k.bin
	./cracker -c tests/01_4c_1k.bin
	./cracker -t 4 -c tests/01_4c_1k.bin tests/02_6c_5.bin
all:
	make cracker
	make tests
clean:
	rm -rf cracker *.bin
