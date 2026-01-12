#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define WORD_LENGTH 9
#define WINDOW_SIZE 18


/* ENUM for error conditions. */
struct lz77_encoder {
	FILE *f;
	char *window;
	char *word;
	char *code;
	int pointer;
	int l;
	int lookahead_index;
	int lookback_index;
};

int log3(int n)
{
	return log(n) / log(3);
}

/*
 * the initial state of the sliding window: WINDOW_SIZE - WORD_LENGTH zeros
 * followed by the first WORD_LENGTH characters from the encoding stream
 */
int lz77_encoder_init(char* filename, struct lz77_encoder *encoder)
{

//	int code_length =
//		log3(WINDOW_SIZE - WORD_LENGTH) + log3(WORD_LENGTH) + 1;
	
	encoder->f = fopen(filename, "r");
	if (encoder->f == NULL)
		return -1;

	encoder->window = malloc(WINDOW_SIZE+1);
	char *wp = encoder->window;

	for (int i = 0; i < WINDOW_SIZE - WORD_LENGTH; i++) {
		*wp = '0';
		wp++;
	}

	int read_amount = fread(wp, sizeof(char), WORD_LENGTH, encoder->f);
	encoder->lookback_index = 0;
	*(wp + read_amount) = '\0';
	if (read_amount < WORD_LENGTH) {
		return 1;
	}
	return 0;
}

int lz77_next_word(struct lz77_encoder *encoder)
{
	int longest_match = 0;
	for (int i = 0; i < WORD_LENGTH; i++) {
		int match = 0;
		char *back = encoder->window + i;
		char *forward = encoder->window + WINDOW_SIZE - WORD_LENGTH;
		while (*back == *forward) {
			match++;
			back++;
			forward++;
		}
		if (match >= longest_match) {
			longest_match = match;
			encoder->pointer = i+1;
		}
	}
	encoder->l = longest_match+1;
	encoder->word = malloc(encoder->l + 1);

	strncpy(encoder->word, encoder->window + WINDOW_SIZE - WORD_LENGTH, longest_match+1);
	*(encoder->word + encoder->l) = '\0';
	return 0;
}	

int lz77_slide_window(struct lz77_encoder *encoder)
{
	char *b = encoder->window;
	char *f = encoder->window + encoder->l;
	while (*f != '\0') {
		*b = *f;
		b++;
		f++;
	}

	int read_amount = fread(b, sizeof(char), encoder->l, encoder->f);
	*(b + read_amount) = '\0';
	if (read_amount < encoder->l) {
		return 1;
	}
	return 0;
}

int lz77_encode_word(struct lz77_encoder *encoder)
{
	
	return 0;
}

int main()
{
	struct lz77_encoder *encoder = malloc(sizeof(struct lz77_encoder));
	if (encoder == NULL)
		return -1;

	lz77_encoder_init("test.txt", encoder);
	lz77_next_word(encoder);
	lz77_slide_window(encoder);

	printf("window -> %s\n", encoder->window);
	printf("word -> %s\n", encoder->word);
	lz77_next_word(encoder);
	lz77_slide_window(encoder);

	printf("window -> %s\n", encoder->window);
	printf("word -> %s\n", encoder->word);

	lz77_next_word(encoder);
	lz77_slide_window(encoder);
	printf("window -> %s\n", encoder->window);
	printf("word -> %s\n", encoder->word);

	lz77_next_word(encoder);
	lz77_slide_window(encoder);
	printf("window -> %s\n", encoder->window);
	printf("word -> %s\n", encoder->word);

	// free the encoder
	return 0;
}
