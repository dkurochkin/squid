#include "squid.h"
#include "errorpage.h"
#include "ssl/ErrorDetail.h"
#if HAVE_MAP
#include <map>
#endif

struct SslErrorEntry {
    Ssl::ssl_error_t value;
    const char *name;
};

static const char *SslErrorDetailDefaultStr = "SSL certificate validation error (%err_name): %ssl_subject";
//Use std::map to optimize search
typedef std::map<Ssl::ssl_error_t, const SslErrorEntry *> SslErrors;
SslErrors TheSslErrors;

static SslErrorEntry TheSslErrorArray[] = {
    {SQUID_X509_V_ERR_DOMAIN_MISMATCH,
        "SQUID_X509_V_ERR_DOMAIN_MISMATCH"},
    {X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT,
     "X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT"},
    {X509_V_ERR_UNABLE_TO_GET_CRL,
     "X509_V_ERR_UNABLE_TO_GET_CRL"},
    {X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE,
     "X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE"},
    {X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE,
     "X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE"},
    {X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY,
     "X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY"},
    {X509_V_ERR_CERT_SIGNATURE_FAILURE,
     "X509_V_ERR_CERT_SIGNATURE_FAILURE"},
    {X509_V_ERR_CRL_SIGNATURE_FAILURE,
     "X509_V_ERR_CRL_SIGNATURE_FAILURE"},
    {X509_V_ERR_CERT_NOT_YET_VALID,
     "X509_V_ERR_CERT_NOT_YET_VALID"},
    {X509_V_ERR_CERT_HAS_EXPIRED,
     "X509_V_ERR_CERT_HAS_EXPIRED"},
    {X509_V_ERR_CRL_NOT_YET_VALID,
     "X509_V_ERR_CRL_NOT_YET_VALID"},
    {X509_V_ERR_CRL_HAS_EXPIRED,
     "X509_V_ERR_CRL_HAS_EXPIRED"},
    {X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD,
     "X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD"},
    {X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD,
     "X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD"},
    {X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD,
     "X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD"},
    {X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD,
     "X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD"},
    {X509_V_ERR_OUT_OF_MEM,
     "X509_V_ERR_OUT_OF_MEM"},
    {X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT,
     "X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT"},
    {X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN,
     "X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN"},
    {X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY,
     "X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY"},
    {X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE,
     "X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE"},
    {X509_V_ERR_CERT_CHAIN_TOO_LONG,
     "X509_V_ERR_CERT_CHAIN_TOO_LONG"},
    {X509_V_ERR_CERT_REVOKED,
     "X509_V_ERR_CERT_REVOKED"},
    {X509_V_ERR_INVALID_CA,
     "X509_V_ERR_INVALID_CA"},
    {X509_V_ERR_PATH_LENGTH_EXCEEDED,
     "X509_V_ERR_PATH_LENGTH_EXCEEDED"},
    {X509_V_ERR_INVALID_PURPOSE,
     "X509_V_ERR_INVALID_PURPOSE"},
    {X509_V_ERR_CERT_UNTRUSTED,
     "X509_V_ERR_CERT_UNTRUSTED"},
    {X509_V_ERR_CERT_REJECTED,
     "X509_V_ERR_CERT_REJECTED"},
    {X509_V_ERR_SUBJECT_ISSUER_MISMATCH,
     "X509_V_ERR_SUBJECT_ISSUER_MISMATCH"},
    {X509_V_ERR_AKID_SKID_MISMATCH,
     "X509_V_ERR_AKID_SKID_MISMATCH"},
    {X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH,
     "X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH"},
    {X509_V_ERR_KEYUSAGE_NO_CERTSIGN,
     "X509_V_ERR_KEYUSAGE_NO_CERTSIGN"},
    {X509_V_ERR_APPLICATION_VERIFICATION,
     "X509_V_ERR_APPLICATION_VERIFICATION"},
    { SSL_ERROR_NONE, "SSL_ERROR_NONE"},
    {SSL_ERROR_NONE, NULL}
};

static void loadSslErrorMap()
{
    assert(TheSslErrors.empty());
    for (int i = 0; TheSslErrorArray[i].name; ++i) {
        TheSslErrors[TheSslErrorArray[i].value] = &TheSslErrorArray[i];
    }
}

Ssl::ssl_error_t Ssl::GetErrorCode(const char *name)
{
    for (int i = 0; TheSslErrorArray[i].name != NULL; i++) {
        if (strcmp(name, TheSslErrorArray[i].name) == 0)
            return TheSslErrorArray[i].value;
    }
    return SSL_ERROR_NONE;
}

Ssl::ssl_error_t
Ssl::ParseErrorString(const char *name)
{
    assert(name);

    const Ssl::ssl_error_t ssl_error = GetErrorCode(name);
    if (ssl_error != SSL_ERROR_NONE)
        return ssl_error;

    if (xisdigit(*name)) {
        const long int value = strtol(name, NULL, 0);
        if (SQUID_SSL_ERROR_MIN <= value && value <= SQUID_SSL_ERROR_MAX)
            return value;
        fatalf("Too small or too bug SSL error code '%s'", name);
    }

    fatalf("Unknown SSL error name '%s'", name);
    return SSL_ERROR_SSL; // not reached
}

const char *Ssl::GetErrorName(Ssl::ssl_error_t value)
{
    if (TheSslErrors.empty())
        loadSslErrorMap();

    const SslErrors::const_iterator it = TheSslErrors.find(value);
    if (it != TheSslErrors.end())
        return it->second->name;

    return NULL;
}

const char *
Ssl::GetErrorDescr(Ssl::ssl_error_t value)
{
    return ErrorDetailsManager::GetInstance().getDefaultErrorDescr(value);
}

Ssl::ErrorDetail::err_frm_code Ssl::ErrorDetail::ErrorFormatingCodes[] = {
    {"ssl_subject", &Ssl::ErrorDetail::subject},
    {"ssl_ca_name", &Ssl::ErrorDetail::ca_name},
    {"ssl_cn", &Ssl::ErrorDetail::cn},
    {"ssl_notbefore", &Ssl::ErrorDetail::notbefore},
    {"ssl_notafter", &Ssl::ErrorDetail::notafter},
    {"err_name", &Ssl::ErrorDetail::err_code},
    {"ssl_error_descr", &Ssl::ErrorDetail::err_descr},
    {NULL,NULL}
};

/**
 * The subject of the current certification in text form
 */
const char  *Ssl::ErrorDetail::subject() const
{
    if (!peer_cert)
        return "[Not available]";

    static char tmpBuffer[256]; // A temporary buffer
    X509_NAME_oneline(X509_get_subject_name(peer_cert.get()), tmpBuffer,
                      sizeof(tmpBuffer));
    return tmpBuffer;
}

// helper function to be used with Ssl::matchX509CommonNames
static int copy_cn(void *check_data,  ASN1_STRING *cn_data)
{
    String *str = (String *)check_data;
    if (!str) // no data? abort
        return 0;
    if (str->defined())
        str->append(", ");
    str->append((const char *)cn_data->data, cn_data->length);
    return 1;
}

/**
 * The list with certificates cn and alternate names
 */
const char *Ssl::ErrorDetail::cn() const
{
    if (!peer_cert)
        return "[Not available]";

    static String tmpStr;  ///< A temporary string buffer
    tmpStr.clean();
    Ssl::matchX509CommonNames(peer_cert.get(), &tmpStr, copy_cn);
    return tmpStr.termedBuf();
}

/**
 * The issuer name
 */
const char *Ssl::ErrorDetail::ca_name() const
{
    if (!peer_cert)
        return "[Not available]";

    static char tmpBuffer[256]; // A temporary buffer
    X509_NAME_oneline(X509_get_issuer_name(peer_cert.get()), tmpBuffer, sizeof(tmpBuffer));
    return tmpBuffer;
}

/**
 * The certificate "not before" field
 */
const char *Ssl::ErrorDetail::notbefore() const
{
    if (!peer_cert)
        return "[Not available]";

    static char tmpBuffer[256]; // A temporary buffer
    ASN1_UTCTIME * tm = X509_get_notBefore(peer_cert.get());
    Ssl::asn1timeToString(tm, tmpBuffer, sizeof(tmpBuffer));
    return tmpBuffer;
}

/**
 * The certificate "not after" field
 */
const char *Ssl::ErrorDetail::notafter() const
{
    if (!peer_cert)
        return "[Not available]";

    static char tmpBuffer[256]; // A temporary buffer
    ASN1_UTCTIME * tm = X509_get_notAfter(peer_cert.get());
    Ssl::asn1timeToString(tm, tmpBuffer, sizeof(tmpBuffer));
    return tmpBuffer;
}

/**
 * The string representation of the error_no
 */
const char *Ssl::ErrorDetail::err_code() const
{
    static char tmpBuffer[64];
    // We can use the GetErrorName but using the detailEntry is faster,
    // so try it first.
    const char *err = detailEntry.name.termedBuf();

    // error details not loaded yet or not defined in error_details.txt,
    // try the GetErrorName...
    if (!err)
        err = GetErrorName(error_no);

    if (!err) {
        snprintf(tmpBuffer, 64, "%d", (int)error_no);
        err = tmpBuffer;
    }
    return err;
}

/**
 * A short description of the error_no
 */
const char *Ssl::ErrorDetail::err_descr() const
{
    if (error_no == SSL_ERROR_NONE)
        return "[No Error]";
    if (const char *err = detailEntry.descr.termedBuf())
        return err;
    return "[Not available]";
}

/**
 * It converts the code to a string value. Currently the following
 * formating codes are supported:
 * %err_name: The name of the SSL error
 * %ssl_error_descr: A short description of the SSL error
 * %ssl_cn: The comma-separated list of common and alternate names
 * %ssl_subject: The certificate subject
 * %ssl_ca_name: The certificate issuer name
 * %ssl_notbefore: The certificate "not before" field
 * %ssl_notafter: The certificate "not after" field
 \retval  the length of the code (the number of characters will be replaced by value)
*/
int Ssl::ErrorDetail::convert(const char *code, const char **value) const
{
    *value = "-";
    for (int i=0; ErrorFormatingCodes[i].code!=NULL; i++) {
        const int len = strlen(ErrorFormatingCodes[i].code);
        if (strncmp(code,ErrorFormatingCodes[i].code, len)==0) {
            ErrorDetail::fmt_action_t action  = ErrorFormatingCodes[i].fmt_action;
            *value = (this->*action)();
            return len;
        }
    }
    return 0;
}

/**
 * It uses the convert method to build the string errDetailStr using
 * a template message for the current SSL error. The template messages
 * can also contain normal error pages formating codes.
 * Currently the error template messages are hard-coded
 */
void Ssl::ErrorDetail::buildDetail() const
{
    char const *s = NULL;
    char const *p;
    char const *t;
    int code_len = 0;

    if (ErrorDetailsManager::GetInstance().getErrorDetail(error_no, request.raw(), detailEntry))
        s = detailEntry.detail.termedBuf();

    if (!s)
        s = SslErrorDetailDefaultStr;

    assert(s);
    while ((p = strchr(s, '%'))) {
        errDetailStr.append(s, p - s);
        code_len = convert(++p, &t);
        if (code_len)
            errDetailStr.append(t);
        else
            errDetailStr.append("%");
        s = p + code_len;
    }
    errDetailStr.append(s, strlen(s));
}

const String &Ssl::ErrorDetail::toString() const
{
    if (!errDetailStr.defined())
        buildDetail();
    return errDetailStr;
}

/* We may do not want to use X509_dup but instead
   internal SSL locking:
   CRYPTO_add(&(cert->references),1,CRYPTO_LOCK_X509);
   peer_cert.reset(cert);
*/
Ssl::ErrorDetail::ErrorDetail( Ssl::ssl_error_t err_no, X509 *cert): error_no (err_no)
{
    peer_cert.reset(X509_dup(cert));
    detailEntry.error_no = SSL_ERROR_NONE;
}

Ssl::ErrorDetail::ErrorDetail(Ssl::ErrorDetail const &anErrDetail)
{
    error_no = anErrDetail.error_no;
    request = anErrDetail.request;

    if (anErrDetail.peer_cert.get()) {
        peer_cert.reset(X509_dup(anErrDetail.peer_cert.get()));
    }

    detailEntry = anErrDetail.detailEntry;
}
