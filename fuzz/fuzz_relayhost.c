/*
 * Fuzz harness for relay host URL parsing.
 *
 * Exercises text_to_relayhost() which parses relay specifications
 * like "smtp://host:port", "smtp+tls://host", "lmtp://host".
 */

#include "includes.h"

#include <stdlib.h>
#include <string.h>

#include "smtpd.h"

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	struct relayhost	 relay;
	char			*buf;

	if (size == 0 || size > 1024)
		return 0;

	buf = malloc(size + 1);
	if (buf == NULL)
		return 0;
	memcpy(buf, data, size);
	buf[size] = '\0';

	text_to_relayhost(&relay, buf);

	free(buf);
	return 0;
}
