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

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <iomanip>
#include <filesystem>
#include <cassert>
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

union LAYOUT
{
    // ファイル・ヘッダ
    struct REC1
    {
        char K_1[ 1];
        char K_2[ 6];
        char K_3[55];
    } r1;
    // 伝票ヘッダ
    struct REC3
    {
        char K_1[ 1];
        char K_2[10];
        char K_3[ 9];
        char K_4[ 9];
        char K_5[ 8];
        char K_6[ 3];
        char K_7[ 6];
        char K_8[16];
    } r3;
    // 伝票明細
    struct REC4
    {
        char K_1[ 1];
        char K_2[ 3];
        char K_3[13];
        char K_4[30];
        char K_5[ 9];
        char K_6[ 6];
    } r4;
    // ファイル・トレーラ
    struct REC9
    {
        char K_1[1];
        char K_2[5];
        char K_3[56];
    } r9;
};

extern void output_header_1(std::ofstream& ofs);

extern void output_header_3(std::ofstream& ofs);

extern void output_header_4(std::ofstream& ofs);

extern void output_header_9(std::ofstream& ofs);

extern char* output_body_1(char* buff, size_t bufl, const LAYOUT::REC1& row);

extern char* output_body_3(char* buff, size_t bufl, const LAYOUT::REC3& row);

extern char* output_body_4(char* buff, size_t bufl, const LAYOUT::REC4& row);

extern char* output_body_9(char* buff, size_t bufl, const LAYOUT::REC9& row);

