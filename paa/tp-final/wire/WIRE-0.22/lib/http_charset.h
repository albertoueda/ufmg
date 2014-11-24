#ifndef _HTTP_CHARSET_H_INCLUDED_
#define _HTTP_CHARSET_H_INCLUDED_

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <errno.h>
#include <iostream>

#include "const.h"

//Para charset detection
#include "nsUniversalDetector.h"

using namespace std;

//Charset Types
enum charset_t {
    CHARSET_UNKNOWN      = 0,
    CHARSET_ISO_8859_1   = 1,
    CHARSET_UTF_8        = 2,
    CHARSET_EUC_KR       = 3,
    CHARSET_ISO_8859_15  = 4,
    CHARSET_WINDOWS_1252 = 5,
    CHARSET_UTF_16       = 6,
    CHARSET_EUC_JP       = 7,
    CHARSET_KOI8_R       = 8,
    CHARSET_KOI8_U       = 9,
    CHARSET_ISO_8859_2   = 10,
    CHARSET_ISO_8859_3   = 11,
    CHARSET_ISO_8859_4   = 12,
    CHARSET_ISO_8859_5   = 13,
    CHARSET_ISO_8859_6   = 14,
    CHARSET_ISO_8859_6_I = 15,
    CHARSET_ISO_8859_6_E = 16,
    CHARSET_ISO_8859_7   = 17,
    CHARSET_ISO_8859_8   = 18,
    CHARSET_ISO_8859_8_I = 19,
    CHARSET_ISO_8859_8_E = 20,
    CHARSET_ISO_8859_9   = 21,
    CHARSET_ISO_8859_10  = 22,
    CHARSET_ISO_8859_11  = 23,
    CHARSET_ISO_8859_13  = 24,
    CHARSET_ISO_8859_14  = 25,
    CHARSET_ISO_8859_16  = 26,
    CHARSET_ISO_IR_111   = 27,
    CHARSET_ISO_2022_CN  = 29,
    CHARSET_ISO_2022_KR  = 30,
    CHARSET_ISO_2022_JP  = 31,
    CHARSET_US_ASCII     = 32,
    CHARSET_UTF_32BE     = 33,
    CHARSET_UTF_32LE     = 34,
    CHARSET_UTF_16BE     = 35,
    CHARSET_UTF_16LE     = 36,
    CHARSET_WINDOWS_1250 = 37,
    CHARSET_WINDOWS_1251 = 38,
    CHARSET_WINDOWS_1253 = 39,
    CHARSET_WINDOWS_1254 = 40,
    CHARSET_WINDOWS_1255 = 41,
    CHARSET_WINDOWS_1256 = 42,
    CHARSET_WINDOWS_1257 = 43,
    CHARSET_WINDOWS_1258 = 44,
    CHARSET_IBM866       = 45,
    CHARSET_IBM850       = 46,
    CHARSET_IBM852       = 47,
    CHARSET_IBM855       = 48,
    CHARSET_IBM857       = 49,
    CHARSET_IBM862       = 50,
    CHARSET_IBM864       = 51,
    CHARSET_IBM864I      = 52,
    CHARSET_UTF_7        = 53,
    CHARSET_SHIFT_JIS    = 54,
    CHARSET_BIG5         = 55,
    CHARSET_GB2312       = 56,
    CHARSET_GB18030      = 57,
    CHARSET_VISCII       = 58,
    CHARSET_TIS_620      = 59,
    CHARSET_HZ_GB_2312   = 61,
    CHARSET_BIG5_HKSCS   = 62,
    CHARSET_X_GBK        = 63,
    CHARSET_X_EUC_TW     = 64
};

#define CHARSET_STR(x) (\
    (x==CHARSET_ISO_8859_1) ? "ISO-8859-1" :\
    (x==CHARSET_UTF_8) ? "UTF-8" :\
    (x==CHARSET_EUC_KR) ? "EUC-KR" :\
    (x==CHARSET_ISO_8859_15) ? "ISO-8859-15" :\
    (x==CHARSET_WINDOWS_1252) ? "windows-1252" :\
    (x==CHARSET_UTF_16) ? "UTF-16" :\
    (x==CHARSET_EUC_JP) ? "EUC-JP" :\
    (x==CHARSET_KOI8_R) ? "KOI8-R" :\
    (x==CHARSET_KOI8_U) ? "KOI8-U" :\
    (x==CHARSET_ISO_8859_2) ? "ISO-8859-2" :\
    (x==CHARSET_ISO_8859_3) ? "ISO-8859-3" :\
    (x==CHARSET_ISO_8859_4) ? "ISO-8859-4" :\
    (x==CHARSET_ISO_8859_5) ? "ISO-8859-5" :\
    (x==CHARSET_ISO_8859_6) ? "ISO-8859-6" :\
    (x==CHARSET_ISO_8859_6_I) ? "ISO-8859-6-I" :\
    (x==CHARSET_ISO_8859_6_E) ? "ISO-8859-6-E" :\
    (x==CHARSET_ISO_8859_7) ? "ISO-8859-7" :\
    (x==CHARSET_ISO_8859_8) ? "ISO-8859-8" :\
    (x==CHARSET_ISO_8859_8_I) ? "ISO-8859-8-I" :\
    (x==CHARSET_ISO_8859_8_E) ? "ISO-8859-8-E" :\
    (x==CHARSET_ISO_8859_9) ? "ISO-8859-9" :\
    (x==CHARSET_ISO_8859_10) ? "ISO-8859-10" :\
    (x==CHARSET_ISO_8859_11) ? "ISO-8859-11" :\
    (x==CHARSET_ISO_8859_13) ? "ISO-8859-13" :\
    (x==CHARSET_ISO_8859_14) ? "ISO-8859-14" :\
    (x==CHARSET_ISO_8859_16) ? "ISO-8859-16" :\
    (x==CHARSET_ISO_IR_111) ? "ISO-IR-111" :\
    (x==CHARSET_ISO_2022_CN) ? "ISO-2022-CN" :\
    (x==CHARSET_ISO_2022_CN) ? "ISO-2022-CN" :\
    (x==CHARSET_ISO_2022_KR) ? "ISO-2022-KR" :\
    (x==CHARSET_ISO_2022_JP) ? "ISO-2022-JP" :\
    (x==CHARSET_US_ASCII) ? "us-ascii" :\
    (x==CHARSET_UTF_32BE) ? "UTF-32BE" :\
    (x==CHARSET_UTF_32LE) ? "UTF-32LE" :\
    (x==CHARSET_UTF_16BE) ? "UTF-16BE" :\
    (x==CHARSET_UTF_16LE) ? "UTF-16LE" :\
    (x==CHARSET_WINDOWS_1250) ? "windows-1250" :\
    (x==CHARSET_WINDOWS_1251) ? "windows-1251" :\
    (x==CHARSET_WINDOWS_1253) ? "windows-1253" :\
    (x==CHARSET_WINDOWS_1254) ? "windows-1254" :\
    (x==CHARSET_WINDOWS_1255) ? "windows-1255" :\
    (x==CHARSET_WINDOWS_1256) ? "windows-1256" :\
    (x==CHARSET_WINDOWS_1257) ? "windows-1257" :\
    (x==CHARSET_WINDOWS_1258) ? "windows-1258" :\
    (x==CHARSET_IBM866) ? "IBM866" :\
    (x==CHARSET_IBM850) ? "IBM850" :\
    (x==CHARSET_IBM852) ? "IBM852" :\
    (x==CHARSET_IBM855) ? "IBM855" :\
    (x==CHARSET_IBM857) ? "IBM857" :\
    (x==CHARSET_IBM862) ? "IBM862" :\
    (x==CHARSET_IBM864) ? "IBM864" :\
    (x==CHARSET_IBM864I) ? "IBM864i" :\
    (x==CHARSET_UTF_7) ? "UTF-7" :\
    (x==CHARSET_SHIFT_JIS) ? "Shift_JIS" :\
    (x==CHARSET_BIG5) ? "Big5" :\
    (x==CHARSET_GB2312) ? "GB2312" :\
    (x==CHARSET_GB18030) ? "gb18030" :\
    (x==CHARSET_VISCII) ? "VISCII" :\
    (x==CHARSET_TIS_620) ? "TIS-620" :\
    (x==CHARSET_HZ_GB_2312) ? "HZ-GB-2312" :\
    (x==CHARSET_BIG5_HKSCS) ? "Big5-HKSCS" :\
    (x==CHARSET_X_GBK) ? "x-gbk" :\
    (x==CHARSET_X_EUC_TW) ? "x-euc-tw" :\
    "undefined")

charset_t http_charset(char *charset_str);
off64_t http_charset_convert(charset_t charset_from, charset_t charset_to, char *src, char *dst);

class wireUniversalDetector : public nsUniversalDetector 
{
    public: 
        wireUniversalDetector();
        ~wireUniversalDetector();
        void Report(const char * aCharset);
        charset_t GetCharset(const char * aBuf, PRUint32 aLen);
    protected:
        charset_t detectedCharset;
        int ready;
};
#endif
