/*
 * Fuzz harness for RFC 5322 email header/body parser.
 *
 * Splits fuzz input into lines and feeds them through the
 * rfc5322_push()/rfc5322_next() state machine, exercising
 * header parsing, continuation folding, and body handling.
 */

#include "includes.h"

#include <stdlib.h>
#include <string.h>

#include "rfc5322.h"

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	struct rfc5322_parser	*parser;
	struct rfc5322_result	 res;
	char			*buf, *line, *next;
	int			 r;

	if (size == 0 || size > 65536)
		return 0;

	parser = rfc5322_parser_new();
	if (parser == NULL)
		return 0;

	buf = malloc(size + 1);
	if (buf == NULL) {
		rfc5322_free(parser);
		return 0;
	}
	memcpy(buf, data, size);
	buf[size] = '\0';

	/* Feed each line to the parser */
	line = buf;
	while (line != NULL) {
		next = strchr(line, '\n');
		if (next != NULL)
			*next++ = '\0';

		if (rfc5322_push(parser, line) == -1)
			break;

		while ((r = rfc5322_next(parser, &res)) != RFC5322_NONE) {
			if (r == RFC5322_ERR || r == RFC5322_END_OF_MESSAGE)
				goto done;
			if (r == RFC5322_HEADER_START)
				rfc5322_unfold_header(parser);
		}
		line = next;
	}

	/* Signal end of message */
	rfc5322_push(parser, NULL);
	while ((r = rfc5322_next(parser, &res)) != RFC5322_NONE) {
		if (r == RFC5322_ERR || r == RFC5322_END_OF_MESSAGE)
			break;
	}

done:
	free(buf);
	rfc5322_free(parser);
	return 0;
}
