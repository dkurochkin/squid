#include "config.h"
#include "format/Tokens.h"
#include "Store.h"

const char *Format::log_tags[] = {
    "NONE",
    "TCP_HIT",
    "TCP_MISS",
    "TCP_REFRESH_UNMODIFIED",
    "TCP_REFRESH_FAIL", // same tag logged for LOG_TCP_REFRESH_FAIL_OLD and
    "TCP_REFRESH_FAIL", // LOG_TCP_REFRESH_FAIL_ERR for backward-compatibility
    "TCP_REFRESH_MODIFIED",
    "TCP_CLIENT_REFRESH_MISS",
    "TCP_IMS_HIT",
    "TCP_SWAPFAIL_MISS",
    "TCP_NEGATIVE_HIT",
    "TCP_MEM_HIT",
    "TCP_DENIED",
    "TCP_DENIED_REPLY",
    "TCP_OFFLINE_HIT",
#if LOG_TCP_REDIRECTS
    "TCP_REDIRECT",
#endif
    "UDP_HIT",
    "UDP_MISS",
    "UDP_DENIED",
    "UDP_INVALID",
    "UDP_MISS_NOFETCH",
    "ICP_QUERY",
    "LOG_TYPE_MAX"
};

// Due to token overlaps between 1 and 2 letter tokens (Bug 3310)
// We split the token table into sets determined by the token length
namespace Format
{

/// 1-char tokens.
static struct TokenTableEntry TokenTable1C[] = {

    {">a", LFT_CLIENT_IP_ADDRESS},
    {">p", LFT_CLIENT_PORT},
    {">A", LFT_CLIENT_FQDN},

    {"<a", LFT_SERVER_IP_ADDRESS},
    {"<p", LFT_SERVER_PORT},
    {"<A", LFT_SERVER_FQDN_OR_PEER_NAME},

    {">h", LFT_REQUEST_HEADER},
    {">h", LFT_REQUEST_ALL_HEADERS},
    {"<h", LFT_REPLY_HEADER},
    {"<h", LFT_REPLY_ALL_HEADERS},

    {">v", LFT_REQUEST_VERSION_OLD_2X},

    {"%", LFT_PERCENT},

    {NULL, LFT_NONE}		/* this must be last */
};

/// 2-char tokens
static struct TokenTableEntry TokenTable2C[] = {

    {">la", LFT_CLIENT_LOCAL_IP},
    {"la", LFT_LOCAL_LISTENING_IP},
    {">lp", LFT_CLIENT_LOCAL_PORT},
    {"lp", LFT_LOCAL_LISTENING_PORT},
    /*{ "lA", LFT_LOCAL_NAME }, */

    {"<la", LFT_SERVER_LOCAL_IP},
    {"oa", LFT_SERVER_LOCAL_IP_OLD_27},
    {"<lp", LFT_SERVER_LOCAL_PORT},
    /* {"ot", LFT_PEER_OUTGOING_TOS}, */

    {"ts", LFT_TIME_SECONDS_SINCE_EPOCH},
    {"tu", LFT_TIME_SUBSECOND},
    {"tl", LFT_TIME_LOCALTIME},
    {"tg", LFT_TIME_GMT},
    {"tr", LFT_TIME_TO_HANDLE_REQUEST},

    {"<pt", LFT_PEER_RESPONSE_TIME},
    {"<tt", LFT_TOTAL_SERVER_SIDE_RESPONSE_TIME},
    {"dt", LFT_DNS_WAIT_TIME},

    {">ha", LFT_ADAPTED_REQUEST_HEADER},
    {">ha", LFT_ADAPTED_REQUEST_ALL_HEADERS},

    {"un", LFT_USER_NAME},
    {"ul", LFT_USER_LOGIN},
    /*{ "ur", LFT_USER_REALM }, */
    /*{ "us", LFT_USER_SCHEME }, */
    {"ui", LFT_USER_IDENT},
    {"ue", LFT_USER_EXTERNAL},

    {"Hs", LFT_HTTP_SENT_STATUS_CODE_OLD_30},
    {">Hs", LFT_HTTP_SENT_STATUS_CODE},
    {"<Hs", LFT_HTTP_RECEIVED_STATUS_CODE},
    /*{ "Ht", LFT_HTTP_STATUS }, */
    {"<bs", LFT_HTTP_BODY_BYTES_READ},

    {"Ss", LFT_SQUID_STATUS},
    {"Sh", LFT_SQUID_HIERARCHY},

    {"mt", LFT_MIME_TYPE},

    {">rm", LFT_CLIENT_REQ_METHOD},
    {">ru", LFT_CLIENT_REQ_URI},
    {">rp", LFT_CLIENT_REQ_URLPATH},
    /*{">rq", LFT_CLIENT_REQ_QUERY},*/
    {">rv", LFT_CLIENT_REQ_VERSION},

    {"rm", LFT_REQUEST_METHOD},
    {"ru", LFT_REQUEST_URI},	/* doesn't include the query-string */
    {"rp", LFT_REQUEST_URLPATH_OLD_31},
    /* { "rq", LFT_REQUEST_QUERY }, * /     / * the query-string, INCLUDING the leading ? */
    {"rv", LFT_REQUEST_VERSION},

    {"<rm", LFT_SERVER_REQ_METHOD},
    {"<ru", LFT_SERVER_REQ_URI},
    {"<rp", LFT_SERVER_REQ_URLPATH},
    /*{"<rq", LFT_SERVER_REQ_QUERY},*/
    {"<rv", LFT_SERVER_REQ_VERSION},

    {">st", LFT_REQUEST_SIZE_TOTAL },
    /*{ ">sl", LFT_REQUEST_SIZE_LINE }, * / / * the request line "GET ... " */
    {">sh", LFT_REQUEST_SIZE_HEADERS },
    /*{ ">sb", LFT_REQUEST_SIZE_BODY }, */
    /*{ ">sB", LFT_REQUEST_SIZE_BODY_NO_TE }, */

    {"<st", LFT_REPLY_SIZE_TOTAL},
    {"<sH", LFT_REPLY_HIGHOFFSET},
    {"<sS", LFT_REPLY_OBJECTSIZE},
    /*{ "<sl", LFT_REPLY_SIZE_LINE }, * /   / * the reply line (protocol, code, text) */
    {"<sh", LFT_REPLY_SIZE_HEADERS },
    /*{ "<sb", LFT_REPLY_SIZE_BODY }, */
    /*{ "<sB", LFT_REPLY_SIZE_BODY_NO_TE }, */

    {"et", LFT_TAG},
    {"st", LFT_IO_SIZE_TOTAL},
    {"ea", LFT_EXT_LOG},
    {"sn", LFT_SEQUENCE_NUMBER},

    {NULL, LFT_NONE}		/* this must be last */
};

#if USE_ADAPTATION
/// Adaptation (adapt::) tokens
static struct TokenTableEntry TokenTableAdapt[] = {
    {"all_trs", LTF_ADAPTATION_ALL_XACT_TIMES},
    {"sum_trs", LTF_ADAPTATION_SUM_XACT_TIMES},
    {"<last_h", LFT_ADAPTATION_LAST_HEADER},
    {NULL, LFT_NONE}		/* this must be last */
};
#endif

#if ICAP_CLIENT
/// ICAP (icap::) tokens
static struct TokenTableEntry TokenTableIcap[] = {
    {"tt", LFT_ICAP_TOTAL_TIME},
    {"<last_h", LFT_ADAPTATION_LAST_HEADER}, // deprecated

    {"<A",  LFT_ICAP_ADDR},
    {"<service_name",  LFT_ICAP_SERV_NAME},
    {"ru",  LFT_ICAP_REQUEST_URI},
    {"rm",  LFT_ICAP_REQUEST_METHOD},
    {">st",  LFT_ICAP_BYTES_SENT},
    {"<st",  LFT_ICAP_BYTES_READ},
    {"<bs", LFT_ICAP_BODY_BYTES_READ},

    {">h",  LFT_ICAP_REQ_HEADER},
    {"<h",  LFT_ICAP_REP_HEADER},

    {"tr",  LFT_ICAP_TR_RESPONSE_TIME},
    {"tio",  LFT_ICAP_IO_TIME},
    {"to",  LFT_ICAP_OUTCOME},
    {"Hs",  LFT_ICAP_STATUS_CODE},

    {NULL, LFT_NONE}		/* this must be last */
};
#endif

/// Miscellaneous >2 byte tokens
static struct TokenTableEntry TokenTableMisc[] = {
    {">eui", LFT_CLIENT_EUI},
    {"err_code", LFT_SQUID_ERROR },
    {"err_detail", LFT_SQUID_ERROR_DETAIL },
    {NULL, LFT_NONE}		/* this must be last */
};

} // namespace Format

/// Scans a token table to see if the next token exists there
/// returns a pointer to next unparsed byte and updates type member if found
char *
Format::Token::scanForToken(const struct TokenTableEntry *table, char *cur)
{
    for (const struct TokenTableEntry *lte = table; lte->config != NULL; lte++) {
        if (strncmp(lte->config, cur, strlen(lte->config)) == 0) {
            type = lte->token_type;
            label = lte->config;
            debugs(46, 7, HERE << "Found token '" << label << "'");
            return cur + strlen(lte->config);
        }
    }
    return cur;
}

/* parses a single token. Returns the token length in characters,
 * and fills in the lt item with the token information.
 * def is for sure null-terminated
 */
int
Format::Token::parse(char *def, Quoting *quoting)
{
    char *cur = def;

    int l;

    l = strcspn(cur, "%");

    if (l > 0) {
        char *cp;
        /* it's a string for sure, until \0 or the next % */
        cp = (char *)xmalloc(l + 1);
        xstrncpy(cp, cur, l + 1);
        type = LFT_STRING;
        data.string = cp;

        while (l > 0) {
            switch (*cur) {

            case '"':

                if (*quoting == LOG_QUOTE_NONE)
                    *quoting = LOG_QUOTE_QUOTES;
                else if (*quoting == LOG_QUOTE_QUOTES)
                    *quoting = LOG_QUOTE_NONE;

                break;

            case '[':
                if (*quoting == LOG_QUOTE_NONE)
                    *quoting = LOG_QUOTE_MIMEBLOB;

                break;

            case ']':
                if (*quoting == LOG_QUOTE_MIMEBLOB)
                    *quoting = LOG_QUOTE_NONE;

                break;
            }

            cur++;
            l--;
        }

        goto done;
    }

    if (!*cur)
        goto done;

    cur++;

    // select quoting style for his particular token
    switch (*cur) {

    case '"':
        quote = LOG_QUOTE_QUOTES;
        cur++;
        break;

    case '\'':
        quote = LOG_QUOTE_RAW;
        cur++;
        break;

    case '[':
        quote = LOG_QUOTE_MIMEBLOB;
        cur++;
        break;

    case '#':
        quote = LOG_QUOTE_URL;
        cur++;
        break;

    default:
        quote = *quoting;
        break;
    }

    if (*cur == '-') {
        left = 1;
        cur++;
    }

    if (*cur == '0') {
        zero = 1;
        cur++;
    }

    if (xisdigit(*cur))
        width = strtol(cur, &cur, 10);

    if (*cur == '.')
        precision = strtol(cur + 1, &cur, 10);

    if (*cur == '{') {
        char *cp;
        cur++;
        l = strcspn(cur, "}");
        cp = (char *)xmalloc(l + 1);
        xstrncpy(cp, cur, l + 1);
        data.string = cp;
        cur += l;

        if (*cur == '}')
            cur++;
    }

    type = LFT_NONE;

    // Scan each token namespace
    if (strncmp(cur, "icap::", 6) == 0) {
#if ICAP_CLIENT
        cur += 6;
        debugs(46, 5, HERE << "scan for icap:: token");
        cur = scanForToken(TokenTableIcap, cur);
#else
        debugs(46, DBG_IMPORTANT, "ERROR: Format uses icap:: token. ICAP disabled!");
#endif
    } else if (strncmp(cur, "adapt::", 7) == 0) {
#if USE_ADAPTATION
        cur += 7;
        debugs(46, 5, HERE << "scan for adapt:: token");
        cur = scanForToken(TokenTableAdapt, cur);
#else
        debugs(46, DBG_IMPORTANT, "ERROR: Format uses adapt:: token. Adaptation disabled!");
#endif
    } else {
        // For upward compatibility, assume "http::" prefix as default prefix
        // for all log access formating codes, except those starting with a
        // "%" or a known namespace. (ie "icap::", "adapt::")
        if (strncmp(cur,"http::", 6) == 0 && *(cur+6) != '%' )
            cur += 6;

        // NP: scan the sets of tokens in decreasing size to guarantee no
        //     mistakes made with overlapping names. (Bug 3310)

        // Scan for various long tokens
        debugs(46, 5, HERE << "scan for possible Misc token");
        cur = scanForToken(TokenTableMisc, cur);
        // scan for 2-char tokens
        if (type == LFT_NONE) {
            debugs(46, 5, HERE << "scan for possible 2C token");
            cur = scanForToken(TokenTable2C, cur);
        }
        // finally scan for 1-char tokens.
        if (type == LFT_NONE) {
            debugs(46, 5, HERE << "scan for possible 1C token");
            cur = scanForToken(TokenTable1C, cur);
        }
    }

    if (type == LFT_NONE) {
        fatalf("Can't parse configuration token: '%s'\n", def);
    }

    if (*cur == ' ') {
        space = 1;
        cur++;
    }

done:

    switch (type) {

#if USE_ADAPTATION
    case LFT_ADAPTATION_LAST_HEADER:
#endif

#if ICAP_CLIENT
    case LFT_ICAP_REQ_HEADER:

    case LFT_ICAP_REP_HEADER:
#endif

    case LFT_ADAPTED_REQUEST_HEADER:

    case LFT_REQUEST_HEADER:

    case LFT_REPLY_HEADER:

        if (data.string) {
            char *header = data.string;
            char *cp = strchr(header, ':');

            if (cp) {
                *cp++ = '\0';

                if (*cp == ',' || *cp == ';' || *cp == ':')
                    data.header.separator = *cp++;
                else
                    data.header.separator = ',';

                data.header.element = cp;

                switch (type) {
                case LFT_REQUEST_HEADER:
                    type = LFT_REQUEST_HEADER_ELEM;
                    break;

                case LFT_ADAPTED_REQUEST_HEADER:
                    type = LFT_ADAPTED_REQUEST_HEADER_ELEM;
                    break;

                case LFT_REPLY_HEADER:
                    type = LFT_REPLY_HEADER_ELEM;
                    break;
#if USE_ADAPTATION
                case LFT_ADAPTATION_LAST_HEADER:
                    type = LFT_ADAPTATION_LAST_HEADER_ELEM;
                    break;
#endif
#if ICAP_CLIENT
                case LFT_ICAP_REQ_HEADER:
                    type = LFT_ICAP_REQ_HEADER_ELEM;
                    break;
                case LFT_ICAP_REP_HEADER:
                    type = LFT_ICAP_REP_HEADER_ELEM;
                    break;
#endif
                default:
                    break;
                }
            }

            data.header.header = header;
        } else {
            switch (type) {
            case LFT_REQUEST_HEADER:
                type = LFT_REQUEST_ALL_HEADERS;
                break;

            case LFT_ADAPTED_REQUEST_HEADER:
                type = LFT_ADAPTED_REQUEST_ALL_HEADERS;
                break;

            case LFT_REPLY_HEADER:
                type = LFT_REPLY_ALL_HEADERS;
                break;
#if USE_ADAPTATION
            case LFT_ADAPTATION_LAST_HEADER:
                type = LFT_ADAPTATION_LAST_ALL_HEADERS;
                break;
#endif
#if ICAP_CLIENT
            case LFT_ICAP_REQ_HEADER:
                type = LFT_ICAP_REQ_ALL_HEADERS;
                break;
            case LFT_ICAP_REP_HEADER:
                type = LFT_ICAP_REP_ALL_HEADERS;
                break;
#endif
            default:
                break;
            }
            Config.onoff.log_mime_hdrs = 1;
        }

        break;

    case LFT_CLIENT_FQDN:
        Config.onoff.log_fqdn = 1;
        break;

    case LFT_TIME_SUBSECOND:
        divisor = 1000;

        if (precision) {
            int i;
            divisor = 1000000;

            for (i = precision; i > 1; i--)
                divisor /= 10;

            if (!divisor)
                divisor = 0;
        }
        break;

    case LFT_HTTP_SENT_STATUS_CODE_OLD_30:
        debugs(46, 0, "WARNING: The \"Hs\" formatting code is deprecated. Use the \">Hs\" instead.");
        type = LFT_HTTP_SENT_STATUS_CODE;
        break;

    case LFT_SERVER_LOCAL_IP_OLD_27:
        debugs(46, 0, "WARNING: The \"oa\" formatting code is deprecated. Use the \"<la\" instead.");
        type = LFT_SERVER_LOCAL_IP;
        break;

    case LFT_REQUEST_URLPATH_OLD_31:
        debugs(46, 0, "WARNING: The \"rp\" formatting code is deprecated. Use the \">rp\" instead.");
        type = LFT_CLIENT_REQ_URLPATH;
        break;

    case LFT_REQUEST_VERSION_OLD_2X:
        debugs(46, 0, "WARNING: The \">v\" formatting code is deprecated. Use the \">rv\" instead.");
        type = LFT_REQUEST_VERSION;
        break;

#if !USE_SQUID_EUI
    case LFT_CLIENT_EUI:
        debugs(46, 0, "WARNING: The \">eui\" formatting code requires EUI features which are disabled in this Squid.");
        break;
#endif

    default:
        break;
    }

    return (cur - def);
}

Format::Token::~Token()
{
    label = NULL; // drop reference to global static.
    safe_free(data.string);
    while (next) {
        Token *tokens = next;
        next = next->next;
        tokens->next = NULL;
        delete tokens;
    }
}

