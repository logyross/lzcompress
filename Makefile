all: main.c 
	cc -g -std=c99 -Wall -Wextra -fsanitize=address -fsanitize=leak main.c -o lzcompress

clean:
	rm lzcompress
