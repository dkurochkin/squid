/*
 * $Id: ACLSslErrorData.cc,v 1.1 2008/02/11 22:24:39 rousskov Exp $
 */

#include "squid.h"
#include "ACLSslErrorData.h"
#include "ACLChecklist.h"
#include "wordlist.h"

ACLSslErrorData::ACLSslErrorData() : values (NULL)
{}

ACLSslErrorData::ACLSslErrorData(ACLSslErrorData const &old) : values (NULL)
{
    assert (!old.values);
}

ACLSslErrorData::~ACLSslErrorData()
{
    if (values)
        delete values;
}

bool
ACLSslErrorData::match(ssl_error_t toFind)
{
    return values->findAndTune (toFind);
}

/* explicit instantiation required for some systems */

template cbdata_type List<ssl_error_t>::CBDATA_List;

wordlist *
ACLSslErrorData::dump()
{
    wordlist *W = NULL;
    List<ssl_error_t> *data = values;

    while (data != NULL) {
        wordlistAdd(&W, sslFindErrorString(data->element));
        data = data->next;
    }

    return W;
}

void
ACLSslErrorData::parse()
{
    List<ssl_error_t> **Tail;
    char *t = NULL;

    for (Tail = &values; *Tail; Tail = &((*Tail)->next))

        ;
    while ((t = strtokFile())) {
        List<ssl_error_t> *q = new List<ssl_error_t>(sslParseErrorString(t));
        *(Tail) = q;
        Tail = &q->next;
    }
}

bool
ACLSslErrorData::empty() const
{
    return values == NULL;
}

ACLData<ssl_error_t> *
ACLSslErrorData::clone() const
{
    /* Splay trees don't clone yet. */
    assert (!values);
    return new ACLSslErrorData(*this);
}