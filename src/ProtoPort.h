
/*
 * $Id: ProtoPort.h,v 1.1 2008/02/11 22:24:39 rousskov Exp $
 */

#ifndef SQUID_PROTO_PORT_H
#define SQUID_PROTO_PORT_H

//#include "typedefs.h"
#include "cbdata.h"

struct http_port_list
{
    http_port_list(const char *aProtocol);
    ~http_port_list();

    http_port_list *next;

    IPAddress s;
    char *protocol;            /* protocol name */
    char *name;                /* visible name */
    char *defaultsite;         /* default web site */

unsigned int transparent:
    1; /* transparent proxy */

unsigned int accel:
    1; /* HTTP accelerator */

unsigned int vhost:
    1; /* uses host header */

unsigned int sslBump:
    1; /* intercepts CONNECT requests */

    int vport;                 /* virtual port support, -1 for dynamic, >0 static*/
    int disable_pmtu_discovery;
#if LINUX_TPROXY
unsigned int tproxy:
    1; /* spoof client ip using tproxy */
#endif

    struct {
	unsigned int enabled;
	unsigned int idle;
	unsigned int interval;
	unsigned int timeout;
    } tcp_keepalive;

#if USE_SSL
     // XXX: temporary hack to ease move of SSL options to http_port
    http_port_list &http; 

    char *cert;
    char *key;
    int version;
    char *cipher;
    char *options;
    char *clientca;
    char *cafile;
    char *capath;
    char *crlfile;
    char *dhfile;
    char *sslflags;
    char *sslcontext;
    SSL_CTX *sslContext;
#endif

    CBDATA_CLASS2(http_port_list);
};


#if USE_SSL

struct https_port_list: public http_port_list
{
    https_port_list();
};

#endif

#endif /* SQUID_PROTO_PORT_H */