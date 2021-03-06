#
#  Makefile for the Squid Portability Library
#

include $(top_srcdir)/src/Common.am


# Ideally this would be 100% inline functions and macro wrappers.

# Port Specific Configurations

noinst_LIBRARIES = libcompat-squid.a
libcompat_squid_a_SOURCES = \
	assert.cc \
	assert.h \
	compat.cc \
	compat.h \
	compat_shared.h \
	cpu.h \
	cppunit.h \
	debug.cc \
	debug.h \
	drand48.h \
	eui64_aton.h \
	fdsetsize.h \
	getaddrinfo.h \
	getnameinfo.h \
	GnuRegex.c \
	GnuRegex.h \
	inet_ntop.h \
	inet_pton.h \
	initgroups.h \
	osdetect.h \
	psignal.h \
	shm.h \
	stdio.h \
	stdvarargs.h \
	strnstr.cc \
	strsep.h \
	strtoll.h \
	tempnam.h \
	types.h \
	unsafe.h \
	valgrind.h \
	xalloc.cc \
	xalloc.h \
	xis.h \
	xstrerror.cc \
	xstrerror.h \
	xstring.cc \
	xstring.h \
	xstrto.cc \
	xstrto.h \
	\
	os/aix.h \
	os/dragonfly.h \
	os/freebsd.h \
	os/hpux.h \
	os/linux.h \
	os/macosx.h \
	os/mswin.h \
	mswin.cc \
	os/netbsd.h \
	os/next.h \
	os/openbsd.h \
	os/opensolaris_10_netdb.h \
	os/os2.h \
	os/qnx.h \
	os/sgi.h \
	os/solaris.h \
	os/sunos.h \
	os/windows.h

libcompat_squid_a_LIBADD= $(LIBOBJS)

check_PROGRAMS += testPreCompiler
TESTS += testPreCompiler

testPreCompiler_SOURCES= \
	testPreCompiler.h \
	testPreCompiler.cc \
	$(top_srcdir)/src/tests/testMain.cc
testPreCompiler_LDADD= $(SQUID_CPPUNIT_LA) $(SQUID_CPPUNIT_LIBS)
testPreCompiler_LDFLAGS=

# os/ subdir prevents us using src/TestHeaders.am
#
TESTS += testHeaders

## Special Universal .h dependency test script
## aborts if error encountered
testHeaders: $(srcdir)/*.h $(srcdir)/os/*.h
	$(SHELL) $(top_srcdir)/test-suite/testheaders.sh "$(CXXCOMPILE)" $^ || exit 1

CLEANFILES += testHeaders

.PHONY: testHeaders
