#include <stdio.h>
#include <stdlib.h>

#define WINDOW_SIZE 12
//#define WINDOW_SIZE 1024
#define SEARCHBUF_SIZE 7
//#define SEARCHBUF_SIZE 512
#define LOOKAHEADBUF_SIZE WINDOW_SIZE - SEARCHBUF_SIZE /* the front end of the sliding window */
//#define LENGTH_BITS 10 /* MUST BE log2(WINDOW_SIZE) rounded up */
//#define OFFSET_BITS 9 /* MUST BE log2(SEARCHBUF_SIZE) rounded up */
#define LENGTH_BITS 4 /* MUST BE log2(WINDOW_SIZE) rounded up */
#define OFFSET_BITS 3 /* MUST BE log2(SEARCHBUF_SIZE) rounded up */
#define SYMBOL_BITS 8;

#define SEARCHBUF_INDEX 0
#define LOOKAHEADBUF_INDEX SEARCHBUF_SIZE

struct lz77_encoder {
	FILE *input_stream;
	FILE *output_stream;
	char *sliding_window;
};

struct lz77_decoder {
	FILE *input_stream;
	FILE *output_stream;
};

struct lz77_encoding_tuple {
	unsigned int offset : OFFSET_BITS;
	unsigned int length : LENGTH_BITS;
	unsigned int symbol : SYMBOL_BITS;
};
	
struct lz77_encoder *lz77_init(char *filename, char *outfile)
{
	struct lz77_encoder *encoder = malloc(sizeof(struct lz77_encoder));
	/* failed to allocate space for encoder. */
	if (encoder == NULL)
		return NULL;

	encoder->input_stream = fopen(filename, "r");
	encoder->output_stream = fopen(outfile, "w");
	encoder->sliding_window = malloc(WINDOW_SIZE+1);

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
	fclose(encoder->output_stream);
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

void lz77_encode(struct lz77_encoding_tuple tup, struct lz77_encoder *encoder)
{
	int encoding_bits = LENGTH_BITS + OFFSET_BITS + SYMBOL_BITS;
	int encoding_bytes = (encoding_bits + 8 - 1) / 8;
	char *payload = malloc(encoding_bytes);
	int symbol_bits = SYMBOL_BITS;
	char *pp = payload;
	int times = encoding_bytes;
	while (times--) {
		*pp = ((((tup.offset << LENGTH_BITS) | tup.length)
			<< symbol_bits) |
		       tup.symbol) >>
		      times * 8;
		/* if (*pp == '\0') */
		/* 	*pp = '\a'; */

		pp++;
	}
	fwrite(payload, sizeof(char), encoding_bytes, encoder->output_stream);
	free(payload);
}

void lz77_compress(char *input_filename, char *output_filename)
{
	struct lz77_encoder *encoder = lz77_init(input_filename,output_filename);
	struct lz77_encoding_tuple tup = lz77_search_match(encoder);
	do{
		//print_sliding_window(encoder);
		//print_encoding_tuple(tup);
		lz77_encode(tup, encoder);
		lz77_slide_window(encoder, tup.length+1);
		tup = lz77_search_match(encoder);
	} while (encoder->sliding_window[LOOKAHEADBUF_INDEX+1] != EOF); 

	lz77_free(encoder);
}

struct lz77_decoder *lz77_decoder_init(char* input_filename, char* output_filename)
{
	struct lz77_decoder *decoder = malloc(sizeof(struct lz77_decoder));
	if (decoder == NULL)
		return NULL;
	decoder->input_stream = fopen(input_filename, "r");
	decoder->output_stream = fopen(output_filename, "w");
	if (decoder->input_stream == NULL || decoder->output_stream == NULL)
		return NULL;

	return decoder;

}
void lz77_decompress(char *input_filename, char *output_filename)
{
	struct lz77_decoder *decoder =
		lz77_decoder_init(input_filename, output_filename);
	int encoding_bits = LENGTH_BITS + OFFSET_BITS + SYMBOL_BITS;
	int encoding_bytes = (encoding_bits + 8 - 1) / 8;
	char encoding[encoding_bytes];
	fread(encoding, sizeof(char), encoding_bytes, decoder->input_stream);
	unsigned char symbol = encoding[encoding_bytes - 1];

	char *ep = encoding;
	unsigned int payload = 0;
	int times = encoding_bytes-1;
	while (times--) {
		payload = (payload << 4) | *ep;
		ep++;
	}
	int offset = payload << OFFSET_BITS;
	int length = payload >> LENGTH_BITS;

}

int main()
{
	//struct lz77_encoder *encoder = lz77_init("bigfile2.txt", "bigfile2.txt.lz");
	//lz77_compress("test.txt", "test.txt.lz");
	lz77_decompress("test.txt.lz", "test.txt.out");
	return 0;
}
