/*
 * Fuzz harness for alias expand node parsing.
 *
 * Exercises text_to_expandnode() which parses alias expansion types:
 * "|command", "/path", ":include:/path", "user@domain",
 * ":error:5xx message", or plain username.
 */

#include "includes.h"

#include <stdlib.h>
#include <string.h>

#include "smtpd.h"

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	struct expandnode	 xn;
	char			*buf;

	if (size == 0 || size > 2048)
		return 0;

	buf = malloc(size + 1);
	if (buf == NULL)
		return 0;
	memcpy(buf, data, size);
	buf[size] = '\0';

	text_to_expandnode(&xn, buf);

	free(buf);
	return 0;
}
