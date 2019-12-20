#ifndef ZIPPER_LIB_H
#define ZIPPER_LIB_H

#include <iostream>
#include <string>
#include <vector>


class Zipper{
public:
    Zipper() = default;

    static void compressFiles(const std::vector<std::string>& fileNames, const std::string targetFile, int level);
    static std::vector<std::string> uncompressFile(const std::string& filename, const std::string& destDir);
};

#endif