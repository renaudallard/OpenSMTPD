#	$OpenBSD$

ASRDIR=		${.CURDIR}/../asr
.PATH:		${.CURDIR}/.. ${ASRDIR}

NEED_ASR!=	grep -q event_asr_run /usr/include/event.h && echo No || echo Yes
NEED_REALLOCARRAY!=	grep -q reallocarray /usr/include/stdlib.h && echo No || echo Yes

PROG=		smtpd

SRCS=		aliases.c
SRCS+=		bounce.c
SRCS+=		ca.c
SRCS+=		compress_backend.c
SRCS+=		config.c
SRCS+=		control.c
SRCS+=		crypto.c
SRCS+=		delivery.c
SRCS+=		dict.c
SRCS+=		dns.c
SRCS+=		envelope.c
SRCS+=		esc.c
SRCS+=		expand.c
SRCS+=		filter.c
SRCS+=		forward.c
SRCS+=		iobuf.c
SRCS+=		ioev.c
SRCS+=		limit.c
SRCS+=		lka.c
SRCS+=		lka_session.c
SRCS+=		log.c
SRCS+=		mda.c
SRCS+=		mproc.c
SRCS+=		mta.c
SRCS+=		mta_session.c
SRCS+=		parse.y
SRCS+=		pony.c
SRCS+=		queue.c
SRCS+=		queue_backend.c
.if ${NEED_REALLOCARRAY} == "Yes"
SRCS+=		reallocarray.c
.endif
SRCS+=		rfc2822.c
SRCS+=		rfc822.c
SRCS+=		ruleset.c
SRCS+=		runq.c
SRCS+=		scheduler.c
SRCS+=		scheduler_backend.c
SRCS+=		smtp.c
SRCS+=		smtp_session.c
SRCS+=		smtpd.c
SRCS+=		ssl.c
SRCS+=		ssl_privsep.c
SRCS+=		ssl_smtpd.c
SRCS+=		stat_backend.c
SRCS+=		table.c
SRCS+=		to.c
SRCS+=		tree.c
SRCS+=		util.c
SRCS+=		waitq.c


# backends
SRCS+=		compress_gzip.c

SRCS+=		delivery_filename.c
SRCS+=		delivery_maildir.c
SRCS+=		delivery_mbox.c
SRCS+=		delivery_mda.c
SRCS+=		delivery_lmtp.c

SRCS+=		table_db.c
SRCS+=		table_getpwnam.c
SRCS+=		table_proc.c
SRCS+=		table_static.c

SRCS+=		queue_fs.c
SRCS+=		queue_null.c
SRCS+=		queue_proc.c
SRCS+=		queue_ram.c
SRCS+=		scheduler_ramqueue.c
SRCS+=		scheduler_null.c
SRCS+=		scheduler_proc.c

SRCS+=		stat_ramstat.c

.if ${NEED_ASR} == "Yes"
CFLAGS+=	-I${ASRDIR}
SRCS+=	asr.c
SRCS+=	asr_debug.c
SRCS+=	asr_private.h
SRCS+=	asr_utils.c
SRCS+=	getaddrinfo.c
SRCS+=	getaddrinfo_async.c
SRCS+=	gethostnamadr.c
SRCS+=	gethostnamadr_async.c
SRCS+=	getnameinfo.c
SRCS+=	getnameinfo_async.c
SRCS+=	getnetnamadr.c
SRCS+=	getnetnamadr_async.c
SRCS+=	getrrsetbyname.c
SRCS+=	getrrsetbyname_async.c
SRCS+=	res_debug.c
SRCS+=	res_init.c
SRCS+=	res_mkquery.c
SRCS+=	res_query.c
SRCS+=	res_search_async.c
SRCS+=	res_send.c
SRCS+=	res_send_async.c
SRCS+=	sethostent.c
SRCS+=	event_asr_run.c
.endif

MAN=		smtpd.8 smtpd.conf.5 table.5
BINDIR=		/usr/sbin

LDADD+=		-levent -lutil -lssl -lcrypto -lm -lz
DPADD+=		${LIBEVENT} ${LIBUTIL} ${LIBSSL} ${LIBCRYPTO} ${LIBM} ${LIBZ}
CFLAGS+=	-g3 -ggdb -I${.CURDIR}/..
CFLAGS+=	-Wall -Wstrict-prototypes -Wmissing-prototypes
CFLAGS+=	-Wmissing-declarations
CFLAGS+=	-Wshadow -Wpointer-arith -Wcast-qual
CFLAGS+=	-Wsign-compare -Wbounded
#CFLAGS+=	-Werror # during development phase (breaks some archs)
CFLAGS+=	-DIO_SSL
CFLAGS+=	-DQUEUE_PROFILING
YFLAGS=

.include <bsd.prog.mk>
