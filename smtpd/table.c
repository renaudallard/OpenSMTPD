/*	$OpenBSD: table.c,v 1.3 2013/02/05 15:23:40 gilles Exp $	*/

/*
 * Copyright (c) 2013 Eric Faurot <eric@openbsd.org>
 * Copyright (c) 2008 Gilles Chehade <gilles@poolp.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/tree.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <event.h>
#include <imsg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "smtpd.h"
#include "log.h"

struct table_backend *table_backend_lookup(const char *);

extern struct table_backend table_backend_static;
extern struct table_backend table_backend_db;
extern struct table_backend table_backend_getpwnam;
extern struct table_backend table_backend_sqlite;
extern struct table_backend table_backend_ldap;

static unsigned int last_table_id = 0;

struct table_backend *
table_backend_lookup(const char *backend)
{
	if (!strcmp(backend, "static") || !strcmp(backend, "file"))
		return &table_backend_static;
	if (!strcmp(backend, "db"))
		return &table_backend_db;
	if (!strcmp(backend, "getpwnam"))
		return &table_backend_getpwnam;
	if (!strcmp(backend, "sqlite"))
		return &table_backend_sqlite;
	if (!strcmp(backend, "ldap"))
		return &table_backend_ldap;
	return NULL;
}

struct table *
table_find(const char *name, const char *tag)
{
	char buf[SMTPD_MAXLINESIZE];

	if (tag == NULL)
		return dict_get(env->sc_tables_dict, name);

	if (snprintf(buf, sizeof(buf), "%s#%s", name, tag) >= (int)sizeof(buf)) {
		log_warnx("warn: table name too long: %s#%s", name, tag);
		return (NULL);
	}

	return dict_get(env->sc_tables_dict, buf);
}

int
table_lookup(struct table *table, const char *key, enum table_service kind,
    void **retp)
{
	return table->t_backend->lookup(table->t_handle, key, kind, retp);
}

int
table_fetch(struct table *table, enum table_service kind, char **retp)
{
	return table->t_backend->fetch(table->t_handle, kind, retp);
}

struct table *
table_create(const char *backend, const char *name, const char *tag,
    const char *config)
{
	struct table		*t;
	struct table_backend	*tb;
	char			 buf[SMTPD_MAXLINESIZE];
	size_t			 n;

	if (name && tag) {
		if (snprintf(buf, sizeof(buf), "%s#%s", name, tag)
		    >= (int)sizeof(buf))
			errx(1, "table_create: name too long \"%s#%s\"",
			    name, tag);
		name = buf;
	}

	if (name && table_find(name, NULL))
		errx(1, "table_create: table \"%s\" already defined", name);

	if ((tb = table_backend_lookup(backend)) == NULL)
		errx(1, "table_create: backend \"%s\" does not exist", backend);

	t = xcalloc(1, sizeof(*t), "table_create");
	t->t_backend = tb;

	/* XXX */
	/*
	 * until people forget about it, "file" really means "static"
	 */
	if (!strcmp(backend, "file"))
		backend = "static";

	if (config) {
		if (strlcpy(t->t_config, config, sizeof t->t_config)
		    >= sizeof t->t_config)
			errx(1, "table_create: table config \"%s\" too large",
			    t->t_config);
	}

	if (strcmp(backend, "static") != 0)
		t->t_type = T_DYNAMIC;

	if (name == NULL)
		snprintf(t->t_name, sizeof(t->t_name), "<dynamic:%u>",
		    last_table_id++);
	else {
		n = strlcpy(t->t_name, name, sizeof(t->t_name));
		if (n >= sizeof(t->t_name))
			errx(1, "table_create: table name too long");
	}

	dict_init(&t->t_dict);
	dict_set(env->sc_tables_dict, t->t_name, t);

	return (t);
}

void
table_destroy(struct table *t)
{
	void	*p = NULL;

	while (dict_poproot(&t->t_dict, NULL, (void **)&p))
		free(p);

	dict_xpop(env->sc_tables_dict, t->t_name);
	free(t);
}

int
table_config(struct table *t)
{
	return (t->t_backend->config(t));
}

void
table_add(struct table *t, const char *key, const char *val)
{
	if (t->t_type & T_DYNAMIC)
		errx(1, "table_add: cannot add to table");
	dict_set(&t->t_dict, key, val ? xstrdup(val, "table_add") : NULL);
}

const void *
table_get(struct table *t, const char *key)
{
	if (t->t_type & T_DYNAMIC)
		errx(1, "table_get: cannot get from table");
	return dict_get(&t->t_dict, key);
}

void
table_delete(struct table *t, const char *key)
{
	if (t->t_type & T_DYNAMIC)
		errx(1, "table_delete: cannot delete from table");
	free(dict_pop(&t->t_dict, key));
}

int
table_check_type(struct table *t, uint32_t mask)
{
	return t->t_type & mask;
}

int
table_check_service(struct table *t, uint32_t mask)
{
	return t->t_backend->services & mask;
}

int
table_check_use(struct table *t, uint32_t tmask, uint32_t smask)
{
	return table_check_type(t, tmask) && table_check_service(t, smask);
}

int
table_open(struct table *t)
{
	t->t_handle = t->t_backend->open(t);
	if (t->t_handle == NULL)
		return 0;
	return 1;
}

void
table_close(struct table *t)
{
	t->t_backend->close(t->t_handle);
}


void
table_update(struct table *t)
{
	t->t_backend->update(t);
}

int
table_domain_match(const char *s1, const char *s2)
{
	return hostname_match(s1, s2);
}

int
table_mailaddr_match(const char *s1, const char *s2)
{
	struct mailaddr m1;
	struct mailaddr m2;

	if (! text_to_mailaddr(&m1, s1))
		return 0;
	if (! text_to_mailaddr(&m2, s2))
		return 0;

	if (strcasecmp(m1.domain, m2.domain))
		return 0;

	if (m2.user[0])
		if (strcasecmp(m1.user, m2.user))
			return 0;
	return 1;
}

static int table_match_mask(struct sockaddr_storage *, struct netaddr *);
static int table_inet4_match(struct sockaddr_in *, struct netaddr *);
static int table_inet6_match(struct sockaddr_in6 *, struct netaddr *);

int
table_netaddr_match(const char *s1, const char *s2)
{
	struct netaddr n1;
	struct netaddr n2;

	if (strcmp(s1, s2) == 0)
		return 1;
	if (! text_to_netaddr(&n1, s1))
		return 0;
	if (! text_to_netaddr(&n2, s2))
		return 0;
	if (n1.ss.ss_family != n2.ss.ss_family)
		return 0;
	if (n1.ss.ss_len != n2.ss.ss_len)
		return 0;
	return table_match_mask(&n1.ss, &n2);
}

static int
table_match_mask(struct sockaddr_storage *ss, struct netaddr *ssmask)
{
	if (ss->ss_family == AF_INET)
		return table_inet4_match((struct sockaddr_in *)ss, ssmask);

	if (ss->ss_family == AF_INET6)
		return table_inet6_match((struct sockaddr_in6 *)ss, ssmask);

	return (0);
}

static int
table_inet4_match(struct sockaddr_in *ss, struct netaddr *ssmask)
{
	in_addr_t mask;
	int i;

	/* a.b.c.d/8 -> htonl(0xff000000) */
	mask = 0;
	for (i = 0; i < ssmask->bits; ++i)
		mask = (mask >> 1) | 0x80000000;
	mask = htonl(mask);

	/* (addr & mask) == (net & mask) */
	if ((ss->sin_addr.s_addr & mask) ==
	    (((struct sockaddr_in *)ssmask)->sin_addr.s_addr & mask))
		return 1;

	return 0;
}

static int
table_inet6_match(struct sockaddr_in6 *ss, struct netaddr *ssmask)
{
	struct in6_addr	*in;
	struct in6_addr	*inmask;
	struct in6_addr	 mask;
	int		 i;

	bzero(&mask, sizeof(mask));
	for (i = 0; i < ssmask->bits / 8; i++)
		mask.s6_addr[i] = 0xff;
	i = ssmask->bits % 8;
	if (i)
		mask.s6_addr[ssmask->bits / 8] = 0xff00 >> i;

	in = &ss->sin6_addr;
	inmask = &((struct sockaddr_in6 *)&ssmask->ss)->sin6_addr;

	for (i = 0; i < 16; i++) {
		if ((in->s6_addr[i] & mask.s6_addr[i]) !=
		    (inmask->s6_addr[i] & mask.s6_addr[i]))
			return (0);
	}

	return (1);
}

void
table_dump_all(void)
{
	struct table	*t;
	void		*iter, *i2;
	const char 	*key, *sep;
	char		*value;
	char		 buf[1024];

	iter = NULL;
	while (dict_iter(env->sc_tables_dict, &iter, NULL, (void **)&t)) {
		i2 = NULL;
		sep = "";
 		buf[0] = '\0';
		if (t->t_type & T_DYNAMIC) {
			strlcat(buf, "DYNAMIC", sizeof(buf));
			sep = ",";
		}
		if (t->t_type & T_LIST) {
			strlcat(buf, sep, sizeof(buf));
			strlcat(buf, "LIST", sizeof(buf));
			sep = ",";
		}
		if (t->t_type & T_HASH) {
			strlcat(buf, sep, sizeof(buf));
			strlcat(buf, "HASH", sizeof(buf));
			sep = ",";
		}
		log_debug("TABLE \"%s\" type=%s config=\"%s\"",
		    t->t_name, buf, t->t_config);
		while(dict_iter(&t->t_dict, &i2, &key, (void**)&value)) {
			if (value)
				log_debug("	\"%s\" -> \"%s\"", key, value);
			else
				log_debug("	\"%s\"", key);
		}
	}
}

void
table_open_all(void)
{
	struct table	*t;
	void		*iter;

	iter = NULL;
	while (dict_iter(env->sc_tables_dict, &iter, NULL, (void **)&t))
		if (! table_open(t))
			errx(1, "failed to open table %s", t->t_name);
}

void
table_close_all(void)
{
	struct table	*t;
	void		*iter;

	iter = NULL;
	while (dict_iter(env->sc_tables_dict, &iter, NULL, (void **)&t))
		table_close(t);
}
