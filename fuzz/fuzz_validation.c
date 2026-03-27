/*
 * Fuzz harness for email validation functions.
 *
 * Exercises valid_localpart(), valid_domainpart(), and valid_xtext()
 * with arbitrary strings.
 */

#include "includes.h"

#include <stdlib.h>
#include <string.h>

#include "smtpd.h"

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	char	*buf;

	if (size == 0 || size > 1024)
		return 0;

	buf = malloc(size + 1);
	if (buf == NULL)
		return 0;
	memcpy(buf, data, size);
	buf[size] = '\0';

	valid_localpart(buf);
	valid_domainpart(buf);
	valid_xtext(buf);

	free(buf);
	return 0;
}
