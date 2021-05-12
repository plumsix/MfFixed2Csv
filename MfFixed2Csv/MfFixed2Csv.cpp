//
// Synopsis
//   マルチフォーマット（マルチレイアウト）の固定長データをCSV高速変換しながら複数ファイルへ仕分け
//
// Usage
//   MfFixed2Csv <IN_FILE> <OUT_DIRECTORY>
//

#include "MfFixed2Csv.h"


int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Error - Too few parameters." << std::endl;
        auto basename = std::filesystem::path(argv[0]);
        std::cerr << basename.stem() << " <IN_FILE> <OUT_DIRECTORY>" << std::endl;
        return -1;
    }
    std::ifstream ifs(argv[1]);
    if (!ifs)
    {
        std::cerr << "Error - No input file." << std::endl;
        return -2;
    }

    struct stat statDirectory;
    if (stat(argv[2], &statDirectory) != 0)
    {
        std::cerr << "Error - Output folder not found." << std::endl;
        return -3;
    }

    std::string o_dir(argv[2]);

    auto in_stem = std::filesystem::path(argv[1]).stem();

    // CSV column names.
    std::ostringstream oss_h;
    oss_h << o_dir << "/" << in_stem << "_HEAD.csv";
    std::ofstream ofs_h(oss_h.str().c_str());
    output_header_h(ofs_h);

    // CSV column names.
    std::ostringstream oss_b;
    oss_b << o_dir << "/" << in_stem << "_BODY.csv";
    std::ofstream ofs_b(oss_b.str().c_str());
    output_header_b(ofs_b);


    return 0;
}
