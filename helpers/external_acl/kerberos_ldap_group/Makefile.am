include $(top_srcdir)/src/Common.am

EXTRA_DIST = README config.test cert_tool ext_kerberos_ldap_group_acl.8
SUBDIRS = 

AM_CPPFLAGS = $(INCLUDES) -I$(srcdir)

libexec_SCRIPTS = cert_tool

libexec_PROGRAMS = ext_kerberos_ldap_group_acl

ext_kerberos_ldap_group_acl_SOURCES = \
	kerberos_ldap_group.cc \
	support.h \
	support_group.cc \
	support_netbios.cc \
	support_member.cc \
	support_krb5.cc \
	support_ldap.cc \
	support_sasl.cc \
	support_resolv.cc \
	support_lserver.cc \
	support_log.cc

ext_kerberos_ldap_group_acl_LDFLAGS =
ext_kerberos_ldap_group_acl_LDADD = \
	$(COMPAT_LIB) \
	$(LDAPLIB) \
	$(LBERLIB) \
	$(LIBSASL) \
	$(KRB5LIBS) \
	$(XTRA_LIBS)
