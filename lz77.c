#include <stdlib.h>

#include "lz77.h"

#define WINDOW_SIZE 5000
#define SEARCHBUF_SIZE 2000
#define LENGTH_BITS 13 /* must be log2(WINDOW_SIZE) rounded up */
#define OFFSET_BITS 11 /* must be log2(SEARCHBUF_SIZE) rounded up */
#define SYMBOL_BITS 8

#define LOOKAHEADBUF_SIZE \
	(WINDOW_SIZE - SEARCHBUF_SIZE) /* the front end of the sliding window */

#define SEARCHBUF_INDEX 0
#define LOOKAHEADBUF_INDEX SEARCHBUF_SIZE

ypedef struct lz77_context {
	FILE *input_stream;
	FILE *output_stream;
	char *sliding_window;

	struct lz77_encoding {
		unsigned int offset : OFFSET_BITS;
		unsigned int length : LENGTH_BITS;
		unsigned int symbol : SYMBOL_BITS;
	} encoding;

	enum lz77_status {
		NO_ERROR,
		MALLOC_ERROR,
		INPUT_STREAM_ERROR,
		OUTPUT_STREAM_ERROR,
		INPUT_EOF
	} status;
} lz77;

lz77 *lz77_init(char *in, char *out, int is_compress)
{
	lz77 *context = malloc(sizeof(lz77));

	if (context == NULL)
		return NULL;

	context->input_stream = fopen(in, "r");
	if (context->input_stream == NULL) {
		context->status = INPUT_STREAM_ERROR;
		return context;
	}

	context->output_stream = fopen(out, "w");
	if (context->output_stream == NULL) {
		context->status = OUTPUT_STREAM_ERROR;
		return context;
	}

	context->sliding_window = malloc(WINDOW_SIZE + 1);

	if (context->sliding_window == NULL) {
		context->status = MALLOC_ERROR;
		return context;
	}

	// for compression fill the lookahead buffer upfront
	if (is_compress) {
		int read_mnt = fread(
			context->sliding_window + LOOKAHEADBUF_INDEX,
			sizeof(char), LOOKAHEADBUF_SIZE, context->input_stream);
		char *start_p =
			context->sliding_window + LOOKAHEADBUF_INDEX + read_mnt;

		char *end_p = context->sliding_window + LOOKAHEADBUF_SIZE +
			      SEARCHBUF_SIZE + 1;

		while (start_p != end_p) {
			*start_p = '\0';
			start_p++;
		}
	}
	context->status = NO_ERROR;

	return context;
}

void lz77_handle_error(enum lz77_status status)
{
	switch (status) {
	case MALLOC_ERROR:
		fprintf(stderr, "Error: Failed to allocate memory.\n");
		break;
	case INPUT_STREAM_ERROR:
		fprintf(stderr, "Error: Failed to open input file.\n");
		break;
	case OUTPUT_STREAM_ERROR:
		fprintf(stderr,
			"Error: Failed to create or open output file.\n");
		break;
	default:
		fprintf(stderr, "Error: unknown error. \n");
	}
}

void lz77_free(lz77 *context)
{
	if (context->sliding_window != NULL)
		free(context->sliding_window);
	if (context->input_stream != NULL)
		fclose(context->input_stream);
	if (context->output_stream != NULL)
		fclose(context->output_stream);

	free(context);
}

void lz77_search(lz77 *context)
{
	int longest_match = 0;
	int longest_match_offset = 0;
	char last_symbol = '\0';

	for (int i = SEARCHBUF_INDEX; i < SEARCHBUF_SIZE; i++) {
		char *lookahead_p =
			context->sliding_window + LOOKAHEADBUF_INDEX;
		char *search_p = context->sliding_window + i;
		int curr_match = 0;
		while (*lookahead_p != '\0' && *lookahead_p == *search_p) {
			lookahead_p++;
			search_p++;
			curr_match++;
		}
		if (*lookahead_p != '\0' && curr_match >= longest_match) {
			longest_match = curr_match;
			longest_match_offset = SEARCHBUF_SIZE - i;
			last_symbol = *lookahead_p;
		}
	}

	if (longest_match == 0)
		longest_match_offset = 0;

	context->encoding.offset = longest_match_offset;
	context->encoding.length = longest_match;
	context->encoding.symbol = last_symbol;
}

void lz77_slide(char *window, int amount)
{
	while (amount-- > 0) {
		for (int i = 0; i < WINDOW_SIZE - 1; i++) {
			window[i] = window[i + 1];
		}
		window[WINDOW_SIZE - 1] = '\0';
	}
}

void lz77_decode(lz77 *context, int times)
{
	times = context->encoding.length;

	while (times--) {
		char c = *(context->sliding_window + WINDOW_SIZE -
			   context->encoding.offset);
		fwrite(&c, sizeof(char), 1, context->output_stream);
		lz77_slide(context->sliding_window, 1);
		context->sliding_window[WINDOW_SIZE - 1] = c;
	}

	lz77_slide(context->sliding_window, 1);
	context->sliding_window[WINDOW_SIZE - 1] = context->encoding.symbol;
	char c = context->encoding.symbol;
	fwrite(&c, sizeof(char), 1, context->output_stream);
}

int lz77_read(lz77 *context, int amount)
{
	char *str = malloc(sizeof(char) * amount + 1);

	int rd = fread(str, sizeof(char), amount, context->input_stream);

	str[rd] = '\0';
	if (rd < amount)
		context->status = INPUT_EOF;

	char *lookahead_buff = context->sliding_window + SEARCHBUF_SIZE +
			       LOOKAHEADBUF_SIZE - amount;

	char *str_to_free = str;
	while (*str != '\0' || *lookahead_buff != '\0') {
		*lookahead_buff = *str;
		str++;
		lookahead_buff++;
	}

	free(str_to_free);
	return rd;
}

int compress(char *in, char *out)
{
	lz77 *encoder = lz77_init(in, out, 1);
	if (encoder->status != NO_ERROR) {
		lz77_handle_error(encoder->status);
		lz77_free(encoder);
		return 1;
	}

	while (*(encoder->sliding_window + LOOKAHEADBUF_INDEX) != '\0') {
		lz77_search(encoder);
		fwrite(&encoder->encoding, sizeof(encoder->encoding), 1,
		       encoder->output_stream);
		lz77_slide(encoder->sliding_window,
			   encoder->encoding.length + 1);
		if (encoder->status != INPUT_EOF)
			lz77_read(encoder, encoder->encoding.length + 1);
	}
	lz77_free(encoder);
	return 0;
}

int decompress(char *in, char *out)
{
	lz77 *decoder = lz77_init(in, out, 0);
	if (decoder->status != NO_ERROR) {
		lz77_handle_error(decoder->status);
		lz77_free(decoder);
		return 1;
	}

	int encoding_bits = LENGTH_BITS + OFFSET_BITS + SYMBOL_BITS;
	int encoding_bytes = (encoding_bits + 8 - 1) / 8;

	while (!feof(decoder->input_stream)) {
		int times = encoding_bytes - 1;
		int read_mnt = fread(&decoder->encoding, sizeof(decoder->encoding),
			1, decoder->input_stream);
		if (read_mnt != 1)
			break;
		lz77_decode(decoder, times);
	}

	lz77_free(decoder);
	return 0;
}
