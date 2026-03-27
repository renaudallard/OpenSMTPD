/*
 * Fuzz harness for user info parsing.
 *
 * Exercises text_to_userinfo() which parses "user:uid:gid:directory"
 * strings into struct userinfo.
 */

#include "includes.h"

#include <stdlib.h>
#include <string.h>

#include "smtpd.h"

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	struct userinfo	 ui;
	char		*buf;

	if (size == 0 || size > 8192)
		return 0;

	buf = malloc(size + 1);
	if (buf == NULL)
		return 0;
	memcpy(buf, data, size);
	buf[size] = '\0';

	text_to_userinfo(&ui, buf);

	free(buf);
	return 0;
}
