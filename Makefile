all: main.c 
	cc -g -std=c99 -Wall -Wextra -Werror -fsanitize=address -fsanitize=leak main.c lz77.c -o lzcompress

clean:
	rm lzcompress
