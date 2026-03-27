/*
 * Fuzz harness for credentials parsing.
 *
 * Exercises text_to_credentials() which parses "user:password"
 * or bare password strings.
 */

#include "includes.h"

#include <stdlib.h>
#include <string.h>

#include "smtpd.h"

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	struct credentials	 creds;
	char			*buf;

	if (size == 0 || size > 1024)
		return 0;

	buf = malloc(size + 1);
	if (buf == NULL)
		return 0;
	memcpy(buf, data, size);
	buf[size] = '\0';

	text_to_credentials(&creds, buf);

	free(buf);
	return 0;
}
