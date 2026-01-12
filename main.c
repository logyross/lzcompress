#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lz77.h"

#define WORD_LENGTH 9
#define WINDOW_SIZE 18
#define FRONT_BUFF WINDOW_SIZE - WORD_LENGTH
#define ALPHABET_SIZE 3
#define CODE_LENGTH 5

char str[] = "001010210210212021021200";

char *n_zeros(int n)
{
	char *s = malloc(n + 1);
	for (int i = 0; i < n; i++)
		s[i] = '0';
	s[n] = '\0';
	return s;
}

char *_strndup(char *s, int n)
{
	char *dst = malloc(n+1);
	strncpy(dst, s, n);
	dst[n] = '\0';
	return dst;
}

char *sliding_window_init(int *stream_index)
{
	char *sliding_window = malloc(WINDOW_SIZE + 1);
	char *zeros = n_zeros(WINDOW_SIZE - WORD_LENGTH);
	strcpy(sliding_window, zeros);
	free(zeros);
	char *slc = _strndup(str, WORD_LENGTH);
	strcat(sliding_window, slc);
	free(slc);
	*stream_index = WORD_LENGTH;
	return sliding_window;
}

char *next_word(char *sliding_window, int *pointer, int *l)
{
	int largest_l = 0;
	for (int i = 0; i < FRONT_BUFF; i++) {
		char *bp = sliding_window + i;
		char *fp = sliding_window + FRONT_BUFF;
		int l = 0;
		while (*bp == *fp) {
			l++;
			bp++;
			fp++;
		}
		if (l >= largest_l) {
			largest_l = l;
			*pointer = i;
		}
	}
	largest_l++;
	(*pointer)++;
	char *word = malloc(largest_l + 1);
	strncpy(word, sliding_window + FRONT_BUFF, largest_l);
	word[largest_l] = '\0';
	*l = largest_l;
	return word;
}


/* char *encode_word(char *word, int pointer, int l) */
char *encode_word()
{
	char *code = malloc(CODE_LENGTH+1);
	return code;
}

void shift_window(char *window, char *stream, int amount)
{
	char *start = window;
	char *end = window + amount;
	while (*end != '\0') {
		*start = *end;
		start++;
		end++;
	}

	while (amount--) {
		*start = *stream;
		start++;
		stream++;
	}
	
}

void encode()
{
	int stream_index = 0;
	// prepare sliding window
	char *sliding_window = sliding_window_init(&stream_index);
	// loop until stream is finished
	int pointer = 0;
	int l = 0;
	char *word = next_word(sliding_window, &pointer, &l);
	///char *code = encode_word(word, pointer, l);
	shift_window(sliding_window, str + stream_index, l);
	// encode word
	// shift buffer
	free(word);
	// loop end
	free(sliding_window);
}

//int main(int argc, char *argv[])
int main()
{
	/* if (argc <= 1) { */
	/* 	fprintf(stderr, "usage: lzcompress FILE"); */
	/* 	return 1; */
	/* } */
	/* FILE *f = fopen(argv[1], "r"); */

	/* if (f == NULL) { */
	/* 	fprintf(stderr, "failed to open file %s\n", argv[1]); */
	/* 	return 1; */
	/* } */

	/* compress(f); */
	/* fclose(f);	*/
	encode();
	return 0;
}
