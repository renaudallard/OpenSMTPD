#!/bin/sh
#
# Build all OpenSMTPD fuzz targets with libFuzzer + ASan + UBSan.
#
# Usage: fuzz/build.sh [srcdir]
#   srcdir defaults to the directory containing this script's parent.
#
# The script:
#   1. Bootstraps and configures with Clang
#   2. Compiles openbsd-compat into a static archive
#   3. Compiles needed smtpd source objects
#   4. Links each fuzz harness
#
# All output goes to fuzz/build/.

set -e

SRCDIR="${1:-$(cd "$(dirname "$0")/.." && pwd)}"
cd "$SRCDIR"

SMTPD_SRC="usr.sbin/smtpd"
COMPAT_SRC="openbsd-compat"
FUZZ_DIR="fuzz"
BUILD_DIR="fuzz/build"

CC="${CC:-clang}"

# Sanitizer flags for compiling source objects (no libFuzzer main)
SANITIZE_COMPILE="-fsanitize=fuzzer-no-link,address,undefined"
# Sanitizer flags for linking harnesses (provides libFuzzer main)
SANITIZE_LINK="-fsanitize=fuzzer,address,undefined"

COMMON_CFLAGS="-g -O1 -fno-omit-frame-pointer -fno-sanitize-recover=all"

# Step 1: Bootstrap and configure if needed
if [ ! -f configure ]; then
	echo "=== Running bootstrap ==="
	./bootstrap
fi

if [ ! -f config.h ]; then
	echo "=== Running configure ==="
	CC="$CC" ./configure --with-gnu-ld --sysconfdir=/etc/mail
fi

# Include paths matching mk/smtpd/Makefile.am (. is for config.h)
INC="-I. -I${SMTPD_SRC} -I${COMPAT_SRC} -I${COMPAT_SRC}/libtls"
CFLAGS_OBJ="$COMMON_CFLAGS $SANITIZE_COMPILE $INC -DIO_TLS"

mkdir -p "$BUILD_DIR"

# Step 2: Build openbsd-compat objects into a static archive
echo "=== Building openbsd-compat ==="
COMPAT_OBJS=""
for f in "$COMPAT_SRC"/*.c "$COMPAT_SRC"/libtls/*.c; do
	base="$(basename "$f" .c)"
	obj="$BUILD_DIR/compat_${base}.o"
	# Compile each file; skip files that fail (platform-specific)
	if $CC $COMMON_CFLAGS $SANITIZE_COMPILE -I. -I"$COMPAT_SRC" -I"$COMPAT_SRC/libtls" \
	   -I"$SMTPD_SRC" -DIO_TLS -w -c "$f" -o "$obj" 2>/dev/null; then
		COMPAT_OBJS="$COMPAT_OBJS $obj"
	fi
done
ar rcs "$BUILD_DIR/libcompat.a" $COMPAT_OBJS

# Step 3: Build needed smtpd source objects
echo "=== Building smtpd objects ==="
SMTPD_SRCS="rfc5322.c to.c dict.c envelope.c unpack_dns.c util.c tree.c"
for f in $SMTPD_SRCS; do
	echo "  $f"
	$CC $CFLAGS_OBJ -c "${SMTPD_SRC}/$f" -o "$BUILD_DIR/${f%.c}.o"
done

# Step 4: Build stubs
echo "=== Building stubs ==="
$CC $CFLAGS_OBJ -c "$FUZZ_DIR/stubs.c" -o "$BUILD_DIR/stubs.o"

# Step 5: Link fuzz targets
echo "=== Linking fuzz targets ==="

LDLIBS="-lresolv -lcrypto -lssl -levent -lz"

link_fuzz() {
	target="$1"; shift
	echo "  $target"
	$CC $COMMON_CFLAGS $SANITIZE_LINK \
		$INC -DIO_TLS \
		"$FUZZ_DIR/${target}.c" \
		"$@" \
		"$BUILD_DIR/stubs.o" \
		-L"$BUILD_DIR" -lcompat \
		$LDLIBS \
		-o "$BUILD_DIR/$target"
}

# Each target with its specific object dependencies
link_fuzz fuzz_rfc5322     "$BUILD_DIR/rfc5322.o"
UTIL_OBJS="$BUILD_DIR/util.o $BUILD_DIR/tree.o"

link_fuzz fuzz_mailaddr    "$BUILD_DIR/to.o" $UTIL_OBJS
link_fuzz fuzz_dns_unpack  "$BUILD_DIR/unpack_dns.o"
link_fuzz fuzz_envelope    "$BUILD_DIR/envelope.o" "$BUILD_DIR/dict.o" \
                           "$BUILD_DIR/to.o" $UTIL_OBJS
link_fuzz fuzz_netaddr     "$BUILD_DIR/to.o" $UTIL_OBJS
link_fuzz fuzz_relayhost   "$BUILD_DIR/to.o" $UTIL_OBJS
link_fuzz fuzz_credentials "$BUILD_DIR/to.o" $UTIL_OBJS
link_fuzz fuzz_userinfo    "$BUILD_DIR/to.o" $UTIL_OBJS
link_fuzz fuzz_expandnode  "$BUILD_DIR/to.o" $UTIL_OBJS
link_fuzz fuzz_validation  $UTIL_OBJS

echo "=== All fuzz targets built in $BUILD_DIR/ ==="
ls -la "$BUILD_DIR"/fuzz_*
