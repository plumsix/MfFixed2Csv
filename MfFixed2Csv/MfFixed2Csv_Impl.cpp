/*
 * An implementation for MfFixed2Csv.cpp
 */

#include "MfFixed2Csv.h"

void output_header_h(std::ofstream& ofs)
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

void output_header_b(std::ofstream& ofs)
{
    ofs
        << "\"区分\","
        << "\"明細番号\","
        << "\"商品コード\","
        << "\"商品名\","
        << "\"単価\","
        << "\"数量\"\n";
}

char* output_body_h(char* buff, const LAYOUT::REC3& row)
{
    char* ptr = buff;
    ptr += sprintf(ptr
        , "%.*s,%.*s,%.*s,%.*s,%.*s,%.*s,%.*s\r\n"
        , sizeof(row.K_1), row.K_1
        , sizeof(row.K_2), row.K_2
        , sizeof(row.K_3), row.K_3
        , sizeof(row.K_4), row.K_4
        , sizeof(row.K_5), row.K_5
        , sizeof(row.K_6), row.K_6
        , sizeof(row.K_7), row.K_7
    );
    buff[ptr - buff] = '\0';
    return buff;
}

char* output_body_b(char* buff, const LAYOUT::REC4& row)
{
    char* ptr = buff;
    ptr += sprintf(ptr
        , "%.*s,%.*s,%.*s,%.*s,%.*s,%.*s\r\n"
        , sizeof(row.K_1), row.K_1
        , sizeof(row.K_2), row.K_2
        , sizeof(row.K_3), row.K_3
        , sizeof(row.K_4), row.K_4
        , sizeof(row.K_5), row.K_5
        , sizeof(row.K_6), row.K_6
    );
    buff[ptr - buff] = '\0';
    return buff;
}
