all: main.c 
	cc -g -std=c99 -Wall -Wextra -fsanitize=address -fsanitize=leak main2.c lz77.c -o lzcompress

clean:
	rm lzcompress
