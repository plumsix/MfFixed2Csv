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

#include "MfFixed2Csv.h"

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

    int total_lines = 0;
    int num_lines_3 = 0;
    int num_lines_4 = 0;

    PK_REC3 pk;                  // stores a slip number recently getline() call.
    LAYOUT unified;                       // For acceptance of fixed length data.
    ::memset(&unified, 0, sizeof(LAYOUT));
    char org_csv_1[sizeof(LAYOUT) * 2]; // A working area to organize CSV format.
    char org_csv_9[sizeof(LAYOUT) * 2];
    char org_csv_3[sizeof(LAYOUT) * 2];
    char org_csv_4[sizeof(LAYOUT) * 2];
    for (;;)
    {
        ifs.getline(unified.line_buff, BUFFER_SIZE);
        if (ifs.bad() || ifs.eof()) {
            break;
        }
        switch (unified.line_buff[0])
        {
        case '3':
            ofs_h << output_body_3(org_csv_3, sizeof(org_csv_3), unified.r3, pk);
            num_lines_3++;
            break;
        case '4':
            ofs_b << output_body_4(org_csv_4, sizeof(org_csv_4), unified.r4, pk);
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
                ofs << output_body_1(org_csv_1, sizeof(org_csv_1), unified.r1);
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
                ofs << output_body_9(org_csv_9, sizeof(org_csv_9), unified.r9);
            }
            break;
        default:
            assert(false);  //The process never reaches here. 
            return -1;
        }
        total_lines++;
    }

    std::cout
        << format("Selected records (3)=%d, rec size=%d"
            , num_lines_3, sizeof(LAYOUT::REC3)) << std::endl
        << format("Selected records (4)=%d, rec size=%d"
            , num_lines_4, sizeof(LAYOUT::REC4)) << std::endl
        << format("Total records=%d", total_lines) << std::endl;

    return 0;
}