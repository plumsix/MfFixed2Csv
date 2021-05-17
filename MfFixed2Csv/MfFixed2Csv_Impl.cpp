/*
==========================================================================
MfFixed2Csv.cpp : CSV format converter
A fixed-length file corded in Shift-JIS organized in multi format
(saied also multi layout) get divided into several files
according to a label consists of one or more characters that placed on a row. 
Simulterneously, each lines are transformed into CSV corded in UTF-8.
Usage:
MfFixed2Csv <IN_FILE> <OUT_DIRECTORY>
How to build:
MSBuild MfFixed2Csv.sln -m -verbosity:minimal -target:Build -property:Configuration=Release
--------------------------------------------------------------------------
Copyright (c) 2021 PLUMSIX Co.,Ltd.
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
==========================================================================
*/

/*
 * An implementation for MfFixed2Csv.cpp
 */

#include "MfFixed2Csv.h"
#include "windows.h"

#define SIZEOF32(arg) (static_cast<unsigned int>(sizeof(arg)))

std::filesystem::path REC1::path_to_csv(
    const std::string& o_dir,
    const std::filesystem::path& in_stem
)
{
    std::filesystem::path pt(o_dir);
    pt /= in_stem.string() + "_1.csv";
    return pt;
}

std::filesystem::path REC3::path_to_csv(
    const std::string& o_dir,
    const std::filesystem::path& in_stem
)
{
    std::filesystem::path pt(o_dir);
    pt /= in_stem.string() + "_3.csv";
    return pt;
}

std::filesystem::path REC4::path_to_csv(
    const std::string& o_dir,
    const std::filesystem::path& in_stem
)
{
    std::filesystem::path pt(o_dir);
    pt /= in_stem.string() + "_4.csv";
    return pt;
}

std::filesystem::path REC9::path_to_csv(
    const std::string& o_dir,
    const std::filesystem::path& in_stem
)
{
    std::filesystem::path pt(o_dir);
    pt /= in_stem.string() + "_9.csv";
    return pt;
}

class Converter
{
public:
    static const unsigned char bom[3];
private:
    CRITICAL_SECTION cs_;
    int wchar_buf_len_;
    std::unique_ptr<wchar_t[]> wchar_buf_;
    int char_buf_len_;
    std::unique_ptr<char[]> char_buf_;
    const char* SjisToUtf8_(const char* str);

public:
    Converter()
        : wchar_buf_len_(3072)
        , wchar_buf_(new wchar_t[wchar_buf_len_])
        , char_buf_len_(1024)
        , char_buf_(new char[char_buf_len_])
    {
        ::InitializeCriticalSection(&cs_);
    }
    ~Converter()
    {
        ::DeleteCriticalSection(&cs_);
    }
    static std::string SjisToUtf8(const char* str)
    {
        static Converter* cvt = new Converter();
        return cvt->SjisToUtf8_(str);
    }
};

const char* Converter::SjisToUtf8_(const char* str)
{
    ::EnterCriticalSection(&cs_);
    // Convert sjis to Unicode
    int iLenUnicode = MultiByteToWideChar(
        CP_ACP, 0, str, static_cast<int>(strlen(str)) + 1,
        NULL, 0
    );
    if (wchar_buf_len_ < iLenUnicode)
    {
        wchar_buf_len_ = iLenUnicode;
        wchar_buf_.reset(new wchar_t[wchar_buf_len_]);
    }
    MultiByteToWideChar(
        CP_ACP, 0, str, static_cast<int>(strlen(str)) + 1
        , wchar_buf_.get(), iLenUnicode
    );
    // Ccnvert Unicode to UTF-8
    int iLenUtf8 = WideCharToMultiByte(
        CP_UTF8, 0, wchar_buf_.get(), iLenUnicode, NULL, 0, 
        NULL, NULL
    );
    if (char_buf_len_ < iLenUtf8)
    {
        char_buf_len_ = iLenUtf8;
        char_buf_.reset(new char[char_buf_len_]);
    }
    WideCharToMultiByte(
        CP_UTF8, 0, wchar_buf_.get(), iLenUnicode, 
        char_buf_.get(), iLenUtf8, NULL, NULL
    );
    ::LeaveCriticalSection(&cs_);
    return char_buf_.get();
}

const unsigned char Converter::bom[]
    = { 0xEF, 0xBB, 0xBF };  // BOM for utf-8 encoding.

void REC1::output_header(std::ofstream& ofs)
{
    ofs << Converter::bom
        << Converter::SjisToUtf8("\"区分\",\"作成日付\"\n");
}

void REC3::output_header(std::ofstream& ofs)
{
    ofs << Converter::bom
        << Converter::SjisToUtf8(
            "\"区分\",\"伝票番号\",\"売場\",\"担当職番\",\"顧客ＩＤ\",\"明細数\",\"時刻\"\n"
        );
}

void REC4::output_header(std::ofstream& ofs)
{
    ofs << Converter::bom 
        << Converter::SjisToUtf8(
            "\"区分\",\"伝票番号\",\"明細番号\",\"商品コード\",\"商品名\",\"単価\",\"数量\"\n"
        );
}

void REC9::output_header(std::ofstream& ofs)
{
    ofs << Converter::bom
        << Converter::SjisToUtf8("\"区分\",\"件数\"\n"
    );
}

/// <summary>
/// Convert the read date to a representation (yyyy-mm-dd) 
/// that can be used as a CSV element
/// </summary>
/// <param name="d"></param>
/// <param name="s"></param>
/// <param name="len"></param>
void adapt_date(
    char d[], 
    const char s[], 
    const size_t& len
)
{
    char* dst = d;
    const char* src = s;
    for (size_t i = 0; i < len; ++i)
    {
        if (i == 4 || i == 6)
        {
            *dst++ = '-';
        }
        *dst++ = *src++;
    }
}

/// <summary>
/// Convert the read time to a representation (MM:ss) 
/// that can be used as a CSV element 
/// </summary>
/// <param name="d">[out] Destination of the converted data </param>
/// <param name="s">[in] Source before conversion </param>
/// <param name="len">[in] Source data length </param>
void adapt_time(
    char d[], 
    const char s[], 
    const size_t& len
)
{
    char* dst = d;
    const char* src = s;
    for (size_t i = 0; i < len; ++i)
    {
        if (i == 2 || i == 4)
        {
            *dst++ = ':';
        }
        *dst++ = *src++;
    }
}

/// <summary>
/// Convert the read character string 
/// to a variable length character string 
/// </summary>
/// <param name="d">[out] Destination of the converted data </param>
/// <param name="s">[in] Source before conversion </param>
/// <param name="len">[in] Source data length </param>
/// <returns></returns>
unsigned adapt_varchar(
    char d[], 
    const char s[], 
    const size_t& len
)
{
    size_t wlen = len;
    char* dst = d + len - 1;
    const char* src = s + len - 1;
    bool removing = true;
    /* Processing to eliminate the spaces from the tail. */
    while (src >= &s[0])
    {
        if (*src == ' ' && removing)
        {
            wlen = src - s;
            --dst;
            --src;
            continue;
        }
        else if (*(short*)(src - 1) == ZEN_KAKU_SPACE && removing)
        {
            wlen = src - s - 1;
            dst -= 2;
            src -= 2;
            continue;
        }
        else
        {
            *dst-- = *src--;
            if (removing) removing = false;
        }
    }
    return static_cast<unsigned>(wlen);
}

std::string REC1::output_body(
    char* buff, 
    const size_t& bufl
)
{
    adapt_date(D02, C02, sizeof(C02));

    ::snprintf(buff, bufl
        , "\"%.*s\",\"%.*s\"\n"
        , SIZEOF32(C01), C01
        , SIZEOF32(D02), D02
    );
    return Converter::SjisToUtf8(buff);
}

std::string REC3::output_body(
    char* buff, 
    const size_t& bufl, 
    PK_REC3& pk
)
{
    // Save items to be transferred to slip details 
    // 伝票明細へ移行する項目を保存
    ::memcpy(pk.C02, C02, sizeof(pk.C02));

    adapt_numeric(N06, C06, sizeof(C06));
    adapt_time(T07, C07, sizeof(C07));

   ::snprintf(buff, bufl
        , "\"%.*s\",\"%.*s\",\"%.*s\",\"%.*s\",\"%.*s\",%d,\"%.*s\"\n"
        , SIZEOF32(C01), C01
        , SIZEOF32(C02), C02
        , SIZEOF32(C03), C03
        , SIZEOF32(C04), C04
        , SIZEOF32(C05), C05
        , N06
        , SIZEOF32(T07), T07
    );
    return Converter::SjisToUtf8(buff);
}

std::string REC4::output_body(
    char* buff, 
    const size_t& bufl, 
    const PK_REC3& pk
)
{
    adapt_numeric(N02, C02, sizeof(C02));
    L03 = adapt_varchar(V03, C03, sizeof(C03));
    L04 = adapt_varchar(V04, C04, sizeof(C04));
    adapt_numeric(N05, C05, sizeof(C05));
    adapt_numeric(N06, C06, sizeof(C06));

    ::snprintf(buff, bufl
        , "\"%.*s\",\"%.*s\",%d,\"%.*s\",\"%.*s\",%d,%d\n"
        , SIZEOF32(C01), C01
        , SIZEOF32(pk.C02), pk.C02
        , N02
        , L03, V03
        , L04, V04
        , static_cast<unsigned>(N05)
        , N06
    );
    return Converter::SjisToUtf8(buff);
}

std::string REC9::output_body(
    char* buff, 
    const size_t& bufl
)
{
    adapt_numeric(N02, C02, sizeof(C02));

    ::snprintf(buff, bufl
        , "\"%.*s\",%d\n"
        , SIZEOF32(C01), C01
        , N02
    );
    return Converter::SjisToUtf8(buff);
}
