include $(top_srcdir)/src/Common.am
include $(top_srcdir)/src/TestHeaders.am

noinst_LTLIBRARIES = libsslsquid.la libsslutil.la

EXTRA_PROGRAMS = \
	ssl_crtd

EXTRA_DIST = \
	stub_libsslsquid.cc \
	stub_libsslutil.cc \
	ssl_crtd.8

if USE_SSL_CRTD
SSL_CRTD = ssl_crtd
SSL_CRTD_SOURCE = \
    helper.cc \
    helper.h
else
SSL_CRTD =
SSL_CRTD_SOURCE =
endif

libsslsquid_la_SOURCES = \
	context_storage.cc \
	context_storage.h \
	Config.cc \
	Config.h \
	ErrorDetail.cc \
	ErrorDetail.h \
	ErrorDetailManager.cc \
	ErrorDetailManager.h \
	support.cc \
	support.h

libsslutil_la_SOURCES = \
	gadgets.cc \
	gadgets.h \
	crtd_message.cc \
	crtd_message.h \
	$(SSL_CRTD_SOURCE)

libexec_PROGRAMS = \
	$(SSL_CRTD)

if USE_SSL_CRTD
ssl_crtd_SOURCES = ssl_crtd.cc certificate_db.cc certificate_db.h
ssl_crtd_LDADD = $(SSLLIB) -lsslutil $(COMPAT_LIB)
endif
