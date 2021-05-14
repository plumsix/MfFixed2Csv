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

#include "MfFixed2Csv.h"

const static size_t BUFFER_SIZE = 1000;

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Error - Too few parameters." << std::endl;
        auto basename = std::filesystem::path(argv[0]);
        std::cerr << basename.stem()
            << " <IN_FILE> <OUT_DIRECTORY>" << std::endl;
        return -1;
    }

    std::ifstream ifs(argv[1]);
    if (!ifs)
    {
        std::cerr
            << "Error - No input file. Specified "
            << argv[1] << std::endl;
        return -2;
    }

    struct stat statDirectory;
    if (stat(argv[2], &statDirectory) != 0)
    {
        std::cerr
            << "Error - Output folder not found. Specified "
            << argv[2] << std::endl;
        return -3;
    }

    std::string o_dir(argv[2]);

    // 入力元ファイルのベース名（出力ファイルの接頭辞として利用される）
    // Base name of input source file (used as prefix of output files)
    auto in_stem = std::filesystem::path(argv[1]).stem();

    // CSV column names.
    std::filesystem::path p3(o_dir);
    p3 /= in_stem.string() + "_3.csv";
    std::ofstream ofs_h(p3);
    output_header_3(ofs_h);

    // CSV column names.
    std::filesystem::path p4(o_dir);
    p4 /= in_stem.string() + "_4.csv";
    std::ofstream ofs_b(p4);
    output_header_4(ofs_b);

    std::cout << format(
        "The data before conversion will be read from %s \n"
        "and written to %s after conversion.\n"
        , argv[1], argv[2]
    ) << std::endl;

    std::string line;
    PK_REC3 pk; // stores a slip number read recently from variable "line". 
    LAYOUT layout;
    int total_lines = 0;
    int num_lines_3 = 0;
    int num_lines_4 = 0;
    char* buff_1 = new char[BUFFER_SIZE + 1];
    char* buff_9 = new char[BUFFER_SIZE + 1];
    char* buff_3 = new char[BUFFER_SIZE + 1];
    char* buff_4 = new char[BUFFER_SIZE + 1];
    while (getline(ifs, line))
    {
        size_t length = line.length();
        switch (line[0])
        {
        case '3':
            ::memcpy(&layout, line.c_str(), length);
            ofs_h << output_body_3(buff_3, BUFFER_SIZE, layout.r3, pk);
            num_lines_3++;
            break;
        case '4':
            ::memcpy(&layout, line.c_str(), length);
            ofs_b << output_body_4(buff_4, BUFFER_SIZE, layout.r4, pk);
            num_lines_4++;
            break;
        case '1':
            {
                std::filesystem::path p1(o_dir);
                p1 /= in_stem.string() + "_1.csv";
                std::ofstream ofs(p1);
                if (ofs.fail())
                {
                    std::cerr
                        << "Error - Failed to open file."
                        << std::endl;
                    return -3;
                }
                output_header_1(ofs);
                ::memcpy(&layout, line.c_str(), length);
                ofs << output_body_1(buff_1, BUFFER_SIZE, layout.r1);
            }
            break;
        case '9':
            {
                std::filesystem::path p9(o_dir);
                p9 /= in_stem.string() + "_9.csv";
                std::ofstream ofs(p9);
                if (ofs.fail())
                {
                    std::cerr
                        << "Error - Failed to open file."
                        << std::endl;
                    return -3;
                }
                output_header_9(ofs);
                ::memcpy(&layout, line.c_str(), length);
                ofs << output_body_9(buff_1, BUFFER_SIZE, layout.r9);
            }
            break;
        default:
            assert(false);  //The process never reaches here. 
            break;
        }
        total_lines++;
    }

    delete[] buff_1;
    delete[] buff_9;
    delete[] buff_3;
    delete[] buff_4;

    std::cout
        << format("Selected records (3)=%d, rec size=%d"
            , num_lines_3, sizeof(LAYOUT::REC3)) << std::endl
        << format("Selected records (4)=%d, rec size=%d"
            , num_lines_4, sizeof(LAYOUT::REC4)) << std::endl
        << format("Total records=%d", total_lines) << std::endl;

    return 0;
}