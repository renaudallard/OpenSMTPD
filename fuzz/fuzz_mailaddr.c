/*
 * Fuzz harness for email address parsing.
 *
 * Exercises text_to_mailaddr() which parses email address strings
 * into struct mailaddr (user + domain components).
 */

#include "includes.h"

#include <stdlib.h>
#include <string.h>

#include "smtpd.h"

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	struct mailaddr	 maddr;
	char		*buf;

	if (size == 0 || size > 1024)
		return 0;

	buf = malloc(size + 1);
	if (buf == NULL)
		return 0;
	memcpy(buf, data, size);
	buf[size] = '\0';

	text_to_mailaddr(&maddr, buf);

	free(buf);
	return 0;
}
