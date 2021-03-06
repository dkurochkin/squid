#include "config.h"
#include "ipcache.h"

#define STUB_API "ipcache.cc"
#include "STUB.h"

void ipcache_purgelru(void *) STUB
void ipcache_nbgethostbyname(const char *name, IPH * handler, void *handlerData) STUB
const ipcache_addrs *ipcache_gethostbyname(const char *, int flags) STUB_RETVAL(NULL)
void ipcacheInvalidate(const char *) STUB
void ipcacheInvalidateNegative(const char *) STUB
void ipcache_init(void) STUB
void ipcacheCycleAddr(const char *name, ipcache_addrs *) STUB
void ipcacheMarkBadAddr(const char *name, const Ip::Address &) STUB
void ipcacheMarkGoodAddr(const char *name, const Ip::Address &) STUB
void ipcacheMarkAllGood(const char *name) STUB
void ipcacheFreeMemory(void) STUB
ipcache_addrs *ipcacheCheckNumeric(const char *name) STUB_RETVAL(NULL)
void ipcache_restart(void) STUB
int ipcacheAddEntryFromHosts(const char *name, const char *ipaddr) STUB_RETVAL(-1)
