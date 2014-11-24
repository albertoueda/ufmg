#include "http_charset.h"

//
//// Name: http_charset
////
//// Descriptions:
////    Checks the charset name and assigns a charset_t element to the charset string
////
//// Input:
////   charset_str - charset string returned by the web server
////
//// Return:
////   charset_t identifier of the charset
////
charset_t http_charset(char * charset_str) {
    int len;

    if (charset_str == NULL) {
        return CHARSET_UNKNOWN;
    }
    
    len = strlen(charset_str);

    if (len >= 10 && !strncasecmp(charset_str, "ISO-8859-1", 10)) {
        return CHARSET_ISO_8859_1;
    }
    else if (len >= 5 && !strncasecmp(charset_str, "UTF-8", 5)) {
        return CHARSET_UTF_8;
    }
    else if (len >= 6 && (!strncasecmp(charset_str, "EUC-KR", 6) || !strncasecmp(charset_str, "EUC_KR", 6))) {
        return CHARSET_EUC_KR;
    }
    else if (len >= 5 && !strncasecmp(charset_str, "EUCKR", 5)) {
        return CHARSET_EUC_KR;
    }
    else if (len >= 11 && !strncasecmp(charset_str, "ISO-8859-15", 11)) {
        return CHARSET_ISO_8859_15;
    }
    else if (len >= 12 && !strncasecmp(charset_str, "windows-1252", 12)) {
        return CHARSET_WINDOWS_1252;
    }
    else if (len >= 6 && !strncasecmp(charset_str, "UTF-16", 6)) {
        return CHARSET_UTF_16;
    }
    else if (len >= 6 && (!strncasecmp(charset_str, "EUC_JP", 6) || !strncasecmp(charset_str, "EUC-JP", 6))) {
        return CHARSET_EUC_JP;
    }
    else if (len >= 5 && !strncasecmp(charset_str, "EUCJP", 5)) {
        return CHARSET_EUC_JP;
    }
    else if (len >= 6 && !strncasecmp(charset_str, "KOI8-R", 6)) {
        return CHARSET_KOI8_R;
    }
    else if (len >= 6 && !strncasecmp(charset_str, "KOI8-U", 6)) {
        return CHARSET_KOI8_U;
    }
    else if (len >= 10 && !strncasecmp(charset_str, "ISO-8859-2", 10)) {
        return CHARSET_ISO_8859_2;
    }
    else if (len >= 10 && !strncasecmp(charset_str, "ISO-8859-3", 10)) {
        return CHARSET_ISO_8859_3;
    }
    else if (len >= 10 && !strncasecmp(charset_str, "ISO-8859-4", 10)) {
        return CHARSET_ISO_8859_4;
    }
    else if (len >= 10 && !strncasecmp(charset_str, "ISO-8859-5", 10)) {
        return CHARSET_ISO_8859_5;
    }
    else if (len >= 10 && !strncasecmp(charset_str, "ISO-8859-6", 10)) {
        return CHARSET_ISO_8859_6;
    }
    else if (len >= 12 && !strncasecmp(charset_str, "ISO-8859-6-I", 12)) {
        return CHARSET_ISO_8859_6_I;
    }
    else if (len >= 12 && !strncasecmp(charset_str, "ISO-8859-6-E", 12)) {
        return CHARSET_ISO_8859_6_E;
    }
    else if (len >= 10 && !strncasecmp(charset_str, "ISO-8859-7", 10)) {
        return CHARSET_ISO_8859_7;
    }
    else if (len >= 10 && !strncasecmp(charset_str, "ISO-8859-8", 10)) {
        return CHARSET_ISO_8859_8;
    }
    else if (len >= 12 && !strncasecmp(charset_str, "ISO-8859-8-I", 12)) {
        return CHARSET_ISO_8859_8_I;
    }
    else if (len >= 12 && !strncasecmp(charset_str, "ISO-8859-8-E", 12)) {
        return CHARSET_ISO_8859_8_E;
    }
    else if (len >= 10 && !strncasecmp(charset_str, "ISO-8859-9", 10)) {
        return CHARSET_ISO_8859_9;
    }
    else if (len >= 11 && !strncasecmp(charset_str, "ISO-8859-10", 11)) {
        return CHARSET_ISO_8859_10;
    }
    else if (len >= 11 && !strncasecmp(charset_str, "ISO-8859-11", 11)) {
        return CHARSET_ISO_8859_11;
    }
    else if (len >= 11 && !strncasecmp(charset_str, "ISO-8859-13", 11)) {
        return CHARSET_ISO_8859_13;
    }
    else if (len >= 11 && !strncasecmp(charset_str, "ISO-8859-14", 11)) {
        return CHARSET_ISO_8859_14;
    }
    else if (len >= 11 && !strncasecmp(charset_str, "ISO-8859-16", 11)) {
        return CHARSET_ISO_8859_16;
    }
    else if (len >= 10 && !strncasecmp(charset_str, "ISO-IR-111", 10)) {
        return CHARSET_ISO_IR_111;
    }
    else if (len >= 11 && !strncasecmp(charset_str, "ISO-2022-CN", 11)) {
        return CHARSET_ISO_2022_CN;
    }
    else if (len >= 11 && !strncasecmp(charset_str, "ISO-2022-CN", 11)) {
        return CHARSET_ISO_2022_CN;
    }
    else if (len >= 11 && !strncasecmp(charset_str, "ISO-2022-KR", 11)) {
        return CHARSET_ISO_2022_KR;
    }
    else if (len >= 11 && !strncasecmp(charset_str, "ISO-2022-JP", 11)) {
        return CHARSET_ISO_2022_JP;
    }
    else if (len >= 8 && !strncasecmp(charset_str, "us-ascii", 8)) {
        return CHARSET_US_ASCII;
    }
    else if (len >= 8 && !strncasecmp(charset_str, "UTF-32BE", 8)) {
        return CHARSET_UTF_32BE;
    }
    else if (len >= 8 && !strncasecmp(charset_str, "UTF-32LE", 8)) {
        return CHARSET_UTF_32LE;
    }
    else if (len >= 8 && !strncasecmp(charset_str, "UTF-16BE", 8)) {
        return CHARSET_UTF_16BE;
    }
    else if (len >= 8 && !strncasecmp(charset_str, "UTF-16LE", 8)) {
        return CHARSET_UTF_16LE;
    }
    else if (len >= 12 && !strncasecmp(charset_str, "windows-1250", 12)) {
        return CHARSET_WINDOWS_1250;
    }
    else if (len >= 12 && !strncasecmp(charset_str, "windows-1251", 12)) {
        return CHARSET_WINDOWS_1251;
    }
    else if (len >= 12 && !strncasecmp(charset_str, "windows-1253", 12)) {
        return CHARSET_WINDOWS_1253;
    }
    else if (len >= 12 && !strncasecmp(charset_str, "windows-1254", 12)) {
        return CHARSET_WINDOWS_1254;
    }
    else if (len >= 12 && !strncasecmp(charset_str, "windows-1255", 12)) {
        return CHARSET_WINDOWS_1255;
    }
    else if (len >= 12 && !strncasecmp(charset_str, "windows-1256", 12)) {
        return CHARSET_WINDOWS_1256;
    }
    else if (len >= 12 && !strncasecmp(charset_str, "windows-1257", 12)) {
        return CHARSET_WINDOWS_1257;
    }
    else if (len >= 12 && !strncasecmp(charset_str, "windows-1258", 12)) {
        return CHARSET_WINDOWS_1258;
    }
    else if (len >= 6 && !strncasecmp(charset_str, "IBM866", 6)) {
        return CHARSET_IBM866;
    }
    else if (len >= 6 && !strncasecmp(charset_str, "IBM850", 6)) {
        return CHARSET_IBM850;
    }
    else if (len >= 6 && !strncasecmp(charset_str, "IBM852", 6)) {
        return CHARSET_IBM852;
    }
    else if (len >= 6 && !strncasecmp(charset_str, "IBM855", 6)) {
        return CHARSET_IBM855;
    }
    else if (len >= 6 && !strncasecmp(charset_str, "IBM857", 6)) {
        return CHARSET_IBM857;
    }
    else if (len >= 6 && !strncasecmp(charset_str, "IBM862", 6)) {
        return CHARSET_IBM862;
    }
    else if (len >= 6 && !strncasecmp(charset_str, "IBM864", 6)) {
        return CHARSET_IBM864;
    }
    else if (len >= 7 && !strncasecmp(charset_str, "IBM864i", 7)) {
        return CHARSET_IBM864I;
    }
    else if (len >= 5 && !strncasecmp(charset_str, "UTF-7", 5)) {
        return CHARSET_UTF_7;
    }
    else if (len >= 9 && !strncasecmp(charset_str, "Shift_JIS", 9)) {
        return CHARSET_SHIFT_JIS;
    }
    else if (len >= 4 && !strncasecmp(charset_str, "Big5", 4)) {
        return CHARSET_BIG5;
    }
    else if (len >= 6 && !strncasecmp(charset_str, "GB2312", 6)) {
        return CHARSET_GB2312;
    }
    else if (len >= 7 && !strncasecmp(charset_str, "gb18030", 7)) {
        return CHARSET_GB18030;
    }
    else if (len >= 6 && !strncasecmp(charset_str, "VISCII", 6)) {
        return CHARSET_VISCII;
    }
    else if (len >= 7 && !strncasecmp(charset_str, "TIS-620", 7)) {
        return CHARSET_TIS_620;
    }
    else if (len >= 10 && !strncasecmp(charset_str, "HZ-GB-2312", 10)) {
        return CHARSET_HZ_GB_2312;
    }
    else if (len >= 10 && !strncasecmp(charset_str, "Big5-HKSCS", 10)) {
        return CHARSET_BIG5_HKSCS;
    }
    else if (len >= 5 && !strncasecmp(charset_str, "x-gbk", 5)) {
        return CHARSET_X_GBK;
    }
    else if (len >= 8 && !strncasecmp(charset_str, "x-euc-tw", 8)) {
        return CHARSET_X_EUC_TW;
    }
    else {
        cerr << "[CHARSET '" << charset_str << "' UNKNOWN]";
        return CHARSET_UNKNOWN;
    }
}

//
//// Name: http_charset_convert
////
//// Descriptions:
////   Converts the src text written in the charset from charset into the charset_to charset and leaves the result
////   in the dst string
////
//// Input:
////   charset_from - charset of the source text
////   charset_to   - charset of the target text
////   src          - source text
////   dst          - target variable
////
//// Return:
////   Size of the target text
////
off64_t http_charset_convert(charset_t charset_from, charset_t charset_to, char *src, char *dst) {
    size_t srclen;
    size_t dstlen;
    size_t inleftsize, outleftsize;
    size_t res;				/* report of iconv */
    iconv_t cd = (iconv_t)-1;

    const char *from;
    const char *to;

    char *inptr;
    char *outptr;

    from = CHARSET_STR(charset_from);
    to   = CHARSET_STR(charset_to);


    /* open iconv */
    cd = iconv_open(to, from);
    if (cd==(iconv_t)(-1)) {
        return (off64_t)cd;
    }

    if (!strcasecmp(from, "UCS-2")) {
        inleftsize=2;
    }
    else {
        srclen = (size_t)strlen(src);
        inleftsize = srclen;
    }
    //outleftsize = inleftsize * 4;
    outleftsize = inleftsize * 4 > MAX_DOC_LEN ? MAX_DOC_LEN : inleftsize * 4;
    dstlen  = outleftsize;
    inptr   = src;
    outptr  = dst;

    while(1) {
        res = iconv(cd, &inptr, &inleftsize, &outptr, &outleftsize);
        if (res == (size_t)(-1)) {
            if (errno == EILSEQ) { /* An invalid multibyte sequence has been encountered in the input. */
                /* cerr << "[CHARSET CONVERT: CAN'T CONVERT FROM " << from <<  "]" << endl; */
                /* for 2-byte code incompleteness */
                inptr++;
                inleftsize--;
            }
            else if (errno == EINVAL) { /* An  incomplete  multibyte  sequence  has been encountered in the input. */
                /* cerr << "[CHARSET CONVERT: INCOMPLETE CHAR OR SHIFT SEQUENCE]" << endl; */
                if (inleftsize <= 2) {
                    *outptr = '?';
                    outleftsize--;
                    break;
                }
            }
            else if (errno == E2BIG) {
                /* cerr << "[OUTPUT TOO BIG]" << endl; */
                break;
            }
            *outptr='?';
            outptr++;
            outleftsize--;
            inptr++;
            inleftsize--;
        }
        else break;
    }
    
    dst[dstlen - outleftsize] = '\0';
    /* close iconv */
    iconv_close(cd);
    return (dstlen - outleftsize);
}

/**
 * Definicion de la clase wireUniversalDetector
 **/

wireUniversalDetector::wireUniversalDetector():nsUniversalDetector() {
    ready = 0;
    detectedCharset = CHARSET_UNKNOWN;
}

wireUniversalDetector::~wireUniversalDetector() {
}
    
void wireUniversalDetector::Report(const char * aCharset) {
    detectedCharset = http_charset((char *)aCharset);
    ready = 1;
}

charset_t wireUniversalDetector::GetCharset(const char * aBuf, PRUint32 aLen) {
    HandleData(aBuf, aLen);
    DataEnd();
    if (ready) {
        return detectedCharset;
    }
    //could't find a confident charset
    return CHARSET_UNKNOWN;
}
