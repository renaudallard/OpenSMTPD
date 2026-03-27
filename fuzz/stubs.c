/*
 * Stub implementations for fuzzing OpenSMTPD parsing functions.
 *
 * Provides no-op log functions and abort()-based fatal/fatalx so that
 * the fuzzer detects any unexpected fatal path as a crash.  Defines
 * globals referenced by smtpd object files but unused by the parsers.
 */

#include "includes.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "smtpd.h"
#include "log.h"

/* globals referenced by linked smtpd objects */
enum smtp_proc_type	 smtpd_process;
/* tracing and foreground_log are defined in util.c */
int			 profiling;
struct smtpd		*env;
struct mproc		*p_control;
struct mproc		*p_parent;
struct mproc		*p_lka;
struct mproc		*p_queue;
struct mproc		*p_scheduler;
struct mproc		*p_dispatcher;
struct mproc		*p_ca;
void			(*imsg_callback)(struct mproc *, struct imsg *);

/* log stubs: no-op */
void
log_init(int n_debug, int facility)
{
	(void)n_debug;
	(void)facility;
}

void
log_procinit(const char *procname)
{
	(void)procname;
}

void
log_setverbose(int v)
{
	(void)v;
}

int
log_getverbose(void)
{
	return 0;
}

void
log_warn(const char *fmt, ...)
{
	(void)fmt;
}

void
log_warnx(const char *fmt, ...)
{
	(void)fmt;
}

void
log_info(const char *fmt, ...)
{
	(void)fmt;
}

void
log_debug(const char *fmt, ...)
{
	(void)fmt;
}

void
logit(int pri, const char *fmt, ...)
{
	(void)pri;
	(void)fmt;
}

void
vlog(int pri, const char *fmt, va_list ap)
{
	(void)pri;
	(void)fmt;
	(void)ap;
}

/* io stubs: util.c references these but fuzz targets never call them */
int
io_print(struct io *io, const char *s)
{
	(void)io;
	(void)s;
	return -1;
}

int
io_vprintf(struct io *io, const char *fmt, va_list ap)
{
	(void)io;
	(void)fmt;
	(void)ap;
	return -1;
}

/*
 * fatal/fatalx call abort() so that any unexpected fatal code path
 * is caught by the fuzzer as a crash.  The parsing functions under
 * test should never reach these on invalid input.
 */
__dead void
fatal(const char *fmt, ...)
{
	(void)fmt;
	abort();
}

__dead void
fatalx(const char *fmt, ...)
{
	(void)fmt;
	abort();
}
