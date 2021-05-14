/*
==========================================================================
MfFixed2Csv.cpp : CSV format converter
Used to convert and distribute  fixed-length files organized
in a multi-format (multi-layout) style to CSV files.
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

    const char* SjisToUtf8_(const char* str)
    {
        ::EnterCriticalSection(&cs_);
        // Convert sjis to Unicode
        int iLenUnicode = MultiByteToWideChar(
            CP_ACP, 0, str, static_cast<int>(strlen(str)) + 1, NULL, 0
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
            CP_UTF8, 0, wchar_buf_.get(), iLenUnicode, NULL, 0, NULL, NULL
        );
        if (char_buf_len_ < iLenUtf8)
        {
            char_buf_len_ = iLenUtf8;
            char_buf_.reset(new char[char_buf_len_]);
        }
        WideCharToMultiByte(CP_UTF8, 0, wchar_buf_.get(), iLenUnicode
            , char_buf_.get(), iLenUtf8, NULL, NULL
        );
        ::LeaveCriticalSection(&cs_);
        return char_buf_.get();
    }

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

const unsigned char Converter::bom[] = { 0xEF, 0xBB, 0xBF };  // BOM for utf-8 encoding.


void output_header_1(std::ofstream& ofs)
{
    ofs << Converter::bom
        << Converter::SjisToUtf8("\"区分\",\"作成日付\"\n");
}

void output_header_3(std::ofstream& ofs)
{
    ofs << Converter::bom
        << Converter::SjisToUtf8(
            "\"区分\",\"伝票番号\",\"売場\",\"担当職番\",\"顧客ＩＤ\",\"明細数\",\"時刻\"\n"
        );
}

void output_header_4(std::ofstream& ofs)
{
    ofs << Converter::bom 
        << Converter::SjisToUtf8(
            "\"区分\",\"伝票番号\",\"明細番号\",\"商品コード\",\"商品名\",\"単価\",\"数量\"\n"
        );
}

void output_header_9(std::ofstream& ofs)
{
    ofs << Converter::bom
        << Converter::SjisToUtf8("\"区分\",\"件数\"\n"
    );
}

std::string output_body_1(char* buff, size_t bufl, const LAYOUT::REC1& row)
{
    ADAPT_DATE(D02, C02);

    ::snprintf(buff, bufl
        , "\"%.*s\",\"%.*s\"\n"
        , SIZEOF32(row.C01), row.C01
        , SIZEOF32(row.D02), row.D02
    );
    return Converter::SjisToUtf8(buff);
}

std::string output_body_3(char* buff, size_t bufl, const LAYOUT::REC3& row, PK_REC3& pk)
{
    // 伝票明細へ移行する項目を保存
    // Save items to be transferred to slip details 
    ::memcpy(pk.C02, row.C02, sizeof(pk.C02));

    ADAPT_NUMERIC(N06, C06);
    ADAPT_TIME(T07, C07);

   ::snprintf(buff, bufl
        , "\"%.*s\",\"%.*s\",\"%.*s\",\"%.*s\",\"%.*s\",%d,\"%.*s\"\n"
        , SIZEOF32(row.C01), row.C01
        , SIZEOF32(row.C02), row.C02
        , SIZEOF32(row.C03), row.C03
        , SIZEOF32(row.C04), row.C04
        , SIZEOF32(row.C05), row.C05
        , row.N06
        , SIZEOF32(row.T07), row.T07
    );
    return Converter::SjisToUtf8(buff);
}

std::string output_body_4(char* buff, size_t bufl, const LAYOUT::REC4& row, const PK_REC3& pk)
{
    ADAPT_NUMERIC(N02, C02);
    ADAPT_VARCHAR(V03, C03, L03);
    ADAPT_VARCHAR(V04, C04, L04);
    ADAPT_NUMERIC(N05, C05);
    ADAPT_NUMERIC(N06, C06);

    ::snprintf(buff, bufl
        , "\"%.*s\",\"%.*s\",%d,\"%.*s\",\"%.*s\",%d,%d\n"
        , SIZEOF32(row.C01), row.C01
        , SIZEOF32(pk.C02), pk.C02
        , row.N02
        , row.L03, row.V03
        , row.L04, row.V04
        , row.N05
        , row.N06
    );
    return Converter::SjisToUtf8(buff);
}

std::string output_body_9(char* buff, size_t bufl, const LAYOUT::REC9& row)
{
    ADAPT_NUMERIC(N02, C02);

    ::snprintf(buff, bufl
        , "\"%.*s\",%d\n"
        , SIZEOF32(row.C01), row.C01
        , row.N02
    );
    return Converter::SjisToUtf8(buff);
}
