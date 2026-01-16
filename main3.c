#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WINDOW_SIZE 12
#define SEARCHBUF_SIZE 7 
#define LOOKAHEADBUF_SIZE WINDOW_SIZE - SEARCHBUF_SIZE /* the front end of the sliding window */
#define LENGTH_BITS 4 /* MUST BE log2(WINDOW_SIZE) rounded up */
#define OFFSET_BITS 3 /* MUST BE log2(SEARCHBUF_SIZE) rounded up */

#define SEARCHBUF_INDEX 0
#define LOOKAHEADBUF_INDEX SEARCHBUF_SIZE

struct lz77_encoder {
	FILE *input_stream;
	char *sliding_window;
	int lookahead_offset;
};

struct lz77_encoding_tuple {
	unsigned int offset : OFFSET_BITS;
	unsigned int length : LENGTH_BITS;
	unsigned int symbol : 8;
};
	
struct lz77_encoder *lz77_init(char *filename)
{
	struct lz77_encoder *encoder = malloc(sizeof(struct lz77_encoder));
	/* failed to allocate space for encoder. */
	if (encoder == NULL)
		return NULL;

	encoder->input_stream = fopen(filename, "r");

	encoder->sliding_window = malloc(WINDOW_SIZE+1);
	encoder->lookahead_offset = SEARCHBUF_SIZE;

	/* either failed to open file or failed to allocate space for sliding window */
	if (encoder->input_stream == NULL || encoder->sliding_window == NULL)
		return NULL;

	/* fill search buffer with empty values */
	for (int i = 0; i < SEARCHBUF_SIZE; i++)
		encoder->sliding_window[i] = '\0';

	/* fill lookahead buffer with data from input stream */
	fread(encoder->sliding_window+LOOKAHEADBUF_INDEX,LOOKAHEADBUF_SIZE, 1, encoder->input_stream);

	return encoder;
}

int lz77_free(struct lz77_encoder *encoder)
{
	free(encoder->sliding_window);
	fclose(encoder->input_stream);
	free(encoder);
	return 0;
}

int lz77_slide_window(struct lz77_encoder *encoder, int amount)
{
	while (amount-- > 0) {
		int s;
		s = getc(encoder->input_stream);
		/* end of file before amount reached, return bytes left. */
		for (int i = 0; i < WINDOW_SIZE-1; i++) {
			encoder->sliding_window[i] = encoder->sliding_window[i+1];
		}
		encoder->sliding_window[WINDOW_SIZE - 1] = s;
		encoder->sliding_window[WINDOW_SIZE] = '\0';
	}
	return 0;
}

void print_encoding_tuple(struct lz77_encoding_tuple tuple)
{
	printf("(%d,%d,%c)\n", tuple.offset, tuple.length, tuple.symbol);
}

void print_sliding_window(struct lz77_encoder *encoder)
{
	for (int i = 0; i < SEARCHBUF_SIZE; i++) {
		if (encoder->sliding_window[i] == '\0')
			printf("_");
		else
			printf("%c", encoder->sliding_window[i]);
	}

	printf("|");

	for (int i = LOOKAHEADBUF_INDEX; i < WINDOW_SIZE; i++) {
		printf("%c", encoder->sliding_window[i]);
	}
	printf("\n");
}

struct lz77_encoding_tuple lz77_search_match(struct lz77_encoder *encoder)
{
	int longest_match = 0;
	int longest_match_offset = 0;
	char last_symbol = '\0';
	/* longest occurence of a substring of lookahead buffer inside search buffer. */
	for (int i = SEARCHBUF_INDEX; i < SEARCHBUF_SIZE; i++) {
		char *lookahead_p = encoder->sliding_window + LOOKAHEADBUF_INDEX;
		char *search_p = encoder->sliding_window + i;
		int match = 0;
		while (*lookahead_p != '\0' && *lookahead_p == *search_p) {
			lookahead_p++;
			search_p++;
			match++;
		}
		if (match >= longest_match) {
			longest_match = match;
			longest_match_offset = SEARCHBUF_SIZE - i;
			last_symbol = *lookahead_p;
		}
	}
	if (longest_match == 0)
		longest_match_offset = 0;
	struct lz77_encoding_tuple result = {longest_match_offset, longest_match, last_symbol};
	return result;
}

void lz77_encode(struct lz77_encoding_tuple tup)
{
       printf("%d%d%d",tup.offset, tup.length, tup.symbol);
}

int main()
{
	struct lz77_encoder *encoder = lz77_init("test.txt");
	struct lz77_encoding_tuple tup = lz77_search_match(encoder);
	do{
		//print_sliding_window(encoder);
		//print_encoding_tuple(tup);

		lz77_slide_window(encoder, tup.length+1);
		tup = lz77_search_match(encoder);
		lz77_encode(tup);
	} while (encoder->sliding_window[LOOKAHEADBUF_INDEX+1] != EOF); 

	lz77_free(encoder);
	return 0;
}
