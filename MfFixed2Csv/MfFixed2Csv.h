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

union LAYOUT
{
    struct REC3
    {
        char K_1[2];
        char K_2[1];
        char K_3[2];
        char K_4[5];
        char K_5[2];
        char K_6[2];
        char K_7[1];
        char K_8[2];
    } r1;
    struct REC4
    {
        char K_1[2];
        char K_2[1];
        char K_3[2];
        char K_4[5];
        char K_5[2];
        char K_6[2];
    } r6;
};

extern void output_header_h(std::ofstream& ofs);

extern void output_header_b(std::ofstream& ofs);

extern char* output_body_h(char* buff, const LAYOUT::REC3& row);

extern char* output_body_b(char* buff, const LAYOUT::REC4& row);

