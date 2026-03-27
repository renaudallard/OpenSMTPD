/*
 * Fuzz harness for network address parsing.
 *
 * Exercises text_to_netaddr() which parses IPv4/IPv6 CIDR notation
 * (e.g. "192.0.2.0/24", "IPv6:2001:db8::/32").
 */

#include "includes.h"

#include <stdlib.h>
#include <string.h>

#include "smtpd.h"

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	struct netaddr	 netaddr;
	char		*buf;

	if (size == 0 || size > 512)
		return 0;

	buf = malloc(size + 1);
	if (buf == NULL)
		return 0;
	memcpy(buf, data, size);
	buf[size] = '\0';

	text_to_netaddr(&netaddr, buf);

	free(buf);
	return 0;
}
