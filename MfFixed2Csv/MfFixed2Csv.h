﻿/*
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

// like the printf in C.
template <typename ... Args>
std::string format(const std::string& fmt, Args ... args)
{
    size_t len = snprintf(nullptr, 0, fmt.c_str(), args ...);
    std::vector<char> buf(len + 1);
    snprintf(&buf[0], len + 1, fmt.c_str(), args ...);
    return std::string(&buf[0], &buf[0] + len);
}

#define DATE_LENGTH 10  // Length when the date is expressed as "yyyy-mm-dd" 
#define TIME_LENGTH  8  // Length when the date is expressed as "hh:MM:dd" 
#define ZEN_KAKU_SPACE   0x4081  // 全角空白 (Shift-JIS)
#define BUFFER_SIZE       1000


// Convert the read date to a representation (yyyy-mm-dd) 
// that can be used as a CSV element
#define ADAPT_DATE(d, s) \
{ \
    char* dst = const_cast<char*>(row.d); \
    const char* src = row.s; \
    for (size_t i = 0; i < sizeof(row.s); ++i) \
    { \
        if (i == 4 || i == 6) \
        { \
            *dst++ = '-'; \
        } \
        *dst++ = *src++; \
    } \
}

// Convert the read time to a representation (MM: ss) 
// that can be used as a CSV element 
#define ADAPT_TIME(d, s) \
{ \
    char* dst = const_cast<char*>(row.d); \
    const char* src = row.s; \
    for (size_t i = 0; i < sizeof(row.s); ++i) \
    { \
        if (i == 2 || i == 4) \
        { \
            *dst++ = ':'; \
        } \
        *dst++ = *src++; \
    } \
}

// Convert the read number to a representation (decimal string) 
// that can be used as a CSV element 
#define ADAPT_NUMERIC(d, s) \
{ \
    int& dst = const_cast<int&>(row.d); \
    const char* src = row.s; \
    dst = 0; \
    for (size_t i = 0; i < sizeof(row.s); ++i) \
    { \
        dst *= 10; \
        dst += (*src++) - '0'; \
    } \
}

// Convert the read character string to a variable length character string 
#define ADAPT_VARCHAR(d, s, l) \
{  \
    /* Processing to eliminate the spaces from the tail. */\
    char* dst = const_cast<char*>(row.d) + sizeof(row.d) - 1; \
    const char* src = row.s + sizeof(row.s) - 1; \
    while (src >= &row.s[0]) \
    { \
        if (*src == ' ') \
        { \
            const_cast<unsigned&>(row.l) = static_cast<unsigned>(src - row.s); \
            --dst; \
            --src; \
            continue; \
        } \
        else if (*(short*)(src - 1) == ZEN_KAKU_SPACE) \
        { \
            const_cast<unsigned&>(row.l) = static_cast<unsigned>(src - row.s - 1); \
            dst -= 2; \
            src -= 2; \
            continue; \
        } \
        else \
        { \
            *dst-- = *src--; \
        } \
    } \
}

struct PK_REC3 {
    char C02[10];          // 伝票番号 (Slip number)
};

union LAYOUT
{
    // For acceptance of fixed length data.
    char line_buff[BUFFER_SIZE];

    // ファイル・ヘッダ (File header)
    struct REC1
    {
        char C01[ 1];          // 区分 (Classification)
        char C02[ 8];          // 作成日付 (Date of creation )
        char C03[55];          // FILLER
        // CSVの要素として利用できるよう変換された属性
        // Attribute converted so that
        // it can be used as an element of CSV 
        char D02[DATE_LENGTH]; // 作成日付

    } r1
    ;
    // 伝票ヘッダ (Slip header)
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
        int N06;               // 明細数
        char T07[TIME_LENGTH]; // 時刻
    } r3
    ;
    // 伝票明細 (Slip details)
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
        int N02;               // 明細番号
        int N05;               // 単価
        int N06;               // 数量
        char V03[13];          // 商品コード
        unsigned L03;          // 長さ
        char V04[30];          // 商品名
        unsigned L04;          // 長さ
    } r4
    ;
    // ファイル・トレーラ (File trailer)
    struct REC9
    {
        char C01[ 1];          // 区分 (Classification)
        char C02[ 5];          // 件数 (Number of lines)
        char C03[56];          // FILLER
        // CSVの要素として利用できるよう変換された属性
        // Attribute converted so that 
        // it can be used as an element of CSV 
        int N02;               // 件数
    } r9
    ;
};

extern void output_header_1(std::ofstream& ofs);

extern void output_header_3(std::ofstream& ofs);

extern void output_header_4(std::ofstream& ofs);

extern void output_header_9(std::ofstream& ofs);

extern std::string output_body_1(char* buff, size_t bufl, const LAYOUT::REC1& row);

extern std::string output_body_3(char* buff, size_t bufl, const LAYOUT::REC3& row, PK_REC3& pk);

extern std::string output_body_4(char* buff, size_t bufl, const LAYOUT::REC4& row, const PK_REC3& pk);

extern std::string output_body_9(char* buff, size_t bufl, const LAYOUT::REC9& row);

