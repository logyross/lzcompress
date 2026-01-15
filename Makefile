all: main.c 
	cc -lm -g -std=c99 -Wall -Wextra -Werror -fsanitize=address -fsanitize=leak main3.c -o lzcompress

clean:
	rm lzcompress
