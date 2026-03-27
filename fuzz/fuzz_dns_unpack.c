/*
 * Fuzz harness for DNS packet unpacking.
 *
 * Exercises unpack_init(), unpack_header(), unpack_query(),
 * unpack_rr(), and dname_expand() with raw binary input,
 * including DNS compression pointer handling.
 */

#include "includes.h"

#include <string.h>

#include "unpack_dns.h"

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	struct unpack		 p;
	struct dns_header	 h;
	struct dns_query	 q;
	struct dns_rr		 rr;
	size_t			 newoffset;
	char			 dst[MAXDNAME];
	int			 i, total;

	if (size == 0 || size > 65535)
		return 0;

	/* Parse full DNS packet */
	unpack_init(&p, (const char *)data, size);

	if (unpack_header(&p, &h) == -1)
		goto expand;

	for (i = 0; i < h.qdcount && i < 64; i++) {
		if (unpack_query(&p, &q) == -1)
			break;
	}

	total = h.ancount + h.nscount + h.arcount;
	for (i = 0; i < total && i < 64; i++) {
		if (unpack_rr(&p, &rr) == -1)
			break;
	}

expand:
	/* Exercise dname_expand at various offsets */
	if (size >= 1)
		dname_expand(data, size, 0, &newoffset, dst, sizeof(dst));
	if (size >= 12)
		dname_expand(data, size, 12, &newoffset, dst, sizeof(dst));

	return 0;
}
