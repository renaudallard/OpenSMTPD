/*
 * Fuzz harness for envelope deserialization.
 *
 * Exercises envelope_load_buffer() which parses the on-disk
 * "field: value" envelope format into struct envelope.
 */

#include "includes.h"

#include <stdlib.h>
#include <string.h>

#include "smtpd.h"

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	struct envelope	 ep;
	char		*buf;

	if (size == 0 || size > 16384)
		return 0;

	/* envelope_load_buffer uses strlcpy internally, needs NUL */
	buf = malloc(size + 1);
	if (buf == NULL)
		return 0;
	memcpy(buf, data, size);
	buf[size] = '\0';

	memset(&ep, 0, sizeof(ep));
	envelope_load_buffer(&ep, buf, size);

	free(buf);
	return 0;
}
