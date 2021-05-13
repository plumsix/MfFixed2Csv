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

#define SIZEOF32(arg) (static_cast<unsigned int>(sizeof(arg)))

void output_header_1(std::ofstream& ofs)
{
    ofs
        << "\"区分\","
        << "\"作成日付\"\n";
}

void output_header_3(std::ofstream& ofs)
{
    ofs
        << "\"区分\","
        << "\"伝票番号\","
        << "\"売場\","
        << "\"担当職番\","
        << "\"顧客ＩＤ\","
        << "\"明細数\","
        << "\"時刻\"\n";
}

void output_header_4(std::ofstream& ofs)
{
    ofs
        << "\"区分\","
        << "\"伝票番号\","
        << "\"明細番号\","
        << "\"商品コード\","
        << "\"商品名\","
        << "\"単価\","
        << "\"数量\"\n";
}

void output_header_9(std::ofstream& ofs)
{
    ofs
        << "\"区分\","
        << "\"件数\"\n";
}

char* output_body_1(char* buff, size_t bufl, const LAYOUT::REC1& row)
{
    ADAPT_DATE(D02, C02);

    char* ptr = buff;
    ptr += ::snprintf(ptr, bufl
        , "\"%.*s\",\"%.*s\"\n"
        , SIZEOF32(row.C01), row.C01
        , SIZEOF32(row.D02), row.D02
    );
    buff[ptr - buff] = '\0';
    return buff;
}

char* output_body_3(char* buff, size_t bufl, const LAYOUT::REC3& row, PK_REC3& pk)
{
    // 伝票明細へ移行する項目を保存
    ::memcpy(pk.C02, row.C02, sizeof(pk.C02));

    ADAPT_NUMERIC(N06, C06);
    ADAPT_TIME(T07, C07);

    char* ptr = buff;
    ptr += ::snprintf(ptr, bufl
        , "\"%.*s\",\"%.*s\",\"%.*s\",\"%.*s\",\"%.*s\",%d,\"%.*s\"\n"
        , SIZEOF32(row.C01), row.C01
        , SIZEOF32(row.C02), row.C02
        , SIZEOF32(row.C03), row.C03
        , SIZEOF32(row.C04), row.C04
        , SIZEOF32(row.C05), row.C05
        , row.N06
        , SIZEOF32(row.T07), row.T07
    );
    buff[ptr - buff] = '\0';
    return buff;
}

char* output_body_4(char* buff, size_t bufl, const LAYOUT::REC4& row, const PK_REC3& pk)
{
    ADAPT_NUMERIC(N02, C02);
    ADAPT_VARCHAR(V03, C03, L03);
    ADAPT_VARCHAR(V04, C04, L04);
    ADAPT_NUMERIC(N05, C05);
    ADAPT_NUMERIC(N06, C06);

    char* ptr = buff;
    ptr += ::snprintf(ptr, bufl
        , "\"%.*s\",\"%.*s\",%d,\"%.*s\",\"%.*s\",%d,%d\n"
        , SIZEOF32(row.C01), row.C01
        , SIZEOF32(pk.C02), pk.C02
        , row.N02
        , row.L03, row.V03
        , row.L04, row.V04
        , row.N05
        , row.N06
    );
    buff[ptr - buff] = '\0';
    return buff;
}

char* output_body_9(char* buff, size_t bufl, const LAYOUT::REC9& row)
{
    ADAPT_NUMERIC(N02, C02);

    char* ptr = buff;
    ptr += ::snprintf(ptr, bufl
        , "\"%.*s\",%d\n"
        , SIZEOF32(row.C01), row.C01
        , row.N02
    );
    buff[ptr - buff] = '\0';
    return buff;
}
