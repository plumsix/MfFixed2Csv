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

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <iomanip>
#include <filesystem>
#include <cassert>
#include <ctime>
#include <string.h>

#define DATE_LENGTH 10  // Length when the date is expressed as "yyyy-mm-dd" 
#define TIME_LENGTH  8  // Length when the date is expressed as "hh:MM:dd" 
#define ZEN_KAKU_SPACE   0x4081  // 全角空白 (Shift-JIS)

/**
 * IMPORTANT:
 * Give the excactly length of a fixed length record.
 * To avoid unintentional overwriting of other members who do not share.
 **/
#define BUFFER_SIZE  64

// like the printf in C.
template <typename ... Args>
std::string format(const std::string& fmt, Args ... args)
{
    size_t len = snprintf(nullptr, 0, fmt.c_str(), args ...);
    std::vector<char> buf(len + 1);
    snprintf(&buf[0], len + 1, fmt.c_str(), args ...);
    return std::string(&buf[0], &buf[0] + len);
}

/// <summary>
/// Convert the read number to a representation (decimal string) 
/// that can be used as a CSV element.
/// Examples conversion of signed decimals:
///  Negative decimal number with a minimum digit other than 0 (1 to 9) 
///   "0012Q" => "-120"   
///  Positive decimal number with a minimum digit other than 0 (1 to 9) 
///   "0012H" =>  "128"
///  Negative decimal number with a minimum digit 0
///   "0012}" => "-120"   
///  Positive decimal number with a minimum digit 0 
///   "0012{" =>  "120"
/// </summary>
/// <param name="d">[out] Destination of the converted data </param>
/// <param name="s">[in] Source before conversion </param>
/// <param name="len">[in] Source data length </param>
/// <returns></returns>
template<typename INT>
bool adapt_numeric(INT& dst, const char s[], const size_t& len)
{
    const char* src = s;
    dst = 0;
    bool rc = false;
    for (size_t i = 0; i < len - 1; ++i)
    {
        dst *= 10;
        dst += (*src++) - '0';
    }
    dst *= 10;
    if (*src < 0x40)           // unsgined integer
    {
        dst += (*src++) - '0';
    }
    else if (*src < 0x4a)  // +1 to +9 signed integer
    {
        dst += (*src++) - '@'; // 0x40
    }
    else if (*src < 0x54)  // -9 to -1 signed integer
    {
        dst += (*src++) - 'I'; // 0x49
        dst *= -1;
    }
    else if (*src < 0x7d)  // +0 signed integer
    {
        dst += (*src++) - '{'; // 0x7b
    }
    else if (*src < 0x7e)  // -0 signed integer
    {
        dst += (*src++) - '}'; // 0x7d
        dst *= -1;
    }
    else
    {
        std::cerr 
            << "Encountered an abnormal number." 
            << std::string(s, len) << std::endl;
        rc = true;;
    }
    return rc;
}

struct PK_REC3 {
    char C02[10];          // 伝票番号 (Slip number)
};

struct REC1
{
    char C01[1];          // 区分 (Classification)
    char C02[8];          // 作成日付 (Date of creation )
    char C03[55];          // FILLER

    // CSVの要素として利用できるよう変換された属性
    // Attribute converted so that
    // it can be used as an element of CSV 
    char D02[DATE_LENGTH]; // 作成日付

    // To hide operations on members. 
    std::filesystem::path path_to_csv(
        const std::string& o_dir,
        const std::filesystem::path& in_stem
    );
    void output_header(std::ofstream& ofs);
    std::string output_body(char* buff, const size_t& bufl);
};

struct REC3
{
    char C01[ 1];          // 区分 (Classification)
    char C02[10];          // 伝票番号 (Slip number)
    char C03[ 9];          // 売場 (Sales floor)
    char C04[ 9];          // 担当職番 (Responsible job number)
    char C05[ 8];          // 顧客ＩＤ (Customer ID)
    char C06[ 3];          // 明細数 (Number of items)
    char C07[ 6];          // 時刻 (Times of Day)
    char C08[16];          // FILLER

    // CSVの要素として利用できるよう変換された属性
    // Attribute converted so that 
    // it can be used as an element of CSV 
    short N06;             // 明細数
    char T07[TIME_LENGTH]; // 時刻

    // To hide operations on members. 
    std::filesystem::path path_to_csv(
        const std::string& o_dir,
        const std::filesystem::path& in_stem
    );
    void output_header(std::ofstream& ofs);
    std::string output_body(char* buff, const size_t& bufl, PK_REC3& pk);
};

struct REC4
{
    char C01[ 1];          // 区分 (Classification)
    char C02[ 3];          // 明細番号 (Item number)
    char C03[13];          // 商品コード (Product code)
    char C04[30];          // 商品名 (Product name)
    char C05[ 9];          // 単価 (Unit price)
    char C06[ 6];          // 数量 (Quantity)

    // CSVの要素として利用できるよう変換された属性
    // Attribute converted so that 
    // it can be used as an element of CSV 
    short N02;             // 明細番号
    int64_t N05;           // 単価
    short N06;             // 数量
    char V03[13];          // 商品コード
    unsigned L03;          // 長さ
    char V04[30];          // 商品名
    unsigned L04;          // 長さ

    // To hide operations on members. 
    std::filesystem::path path_to_csv(
        const std::string& o_dir,
        const std::filesystem::path& in_stem
    );
    void output_header(std::ofstream& ofs);
    std::string output_body(char* buff, const size_t& bufl, const PK_REC3& pk);
};

struct REC9
{
    char C01[ 1];          // 区分 (Classification)
    char C02[ 5];          // 件数 (Number of lines)
    char C03[56];          // FILLER

    // CSVの要素として利用できるよう変換された属性
    // Attribute converted so that 
    // it can be used as an element of CSV 
    short N02;             // 件数

    // To hide operations on members. 
    std::filesystem::path path_to_csv(
        const std::string& o_dir,
        const std::filesystem::path& in_stem
    );
    void output_header(std::ofstream& ofs);
    std::string output_body(char* buff, const size_t& bufl);
};

union LAYOUT
{
    // For acceptance of fixed length data.
    char line_buff[BUFFER_SIZE];
    // ファイル・ヘッダ (File header)
    REC1 r1;
    // 伝票ヘッダ (Slip header)
    REC3 r3;
    // 伝票明細 (Slip details)
    REC4 r4;
    // ファイル・トレーラ (File trailer)
    REC9 r9;

    LAYOUT()
    {
        ::memset(this, 0, sizeof(*this));
    }
};
