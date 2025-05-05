//
// Created by raffael on 04.05.25.
//

#include "utility/FileSystemAbstraction.h"

#include <fstream>
#include <string>
#include <experimental/filesystem>

std::string FileSystemAbstraction::LoadFile(const std::string &filePath) {
 std::ifstream ifs(filePath);
 std::string content((std::istreambuf_iterator<char>(ifs)),
                     (std::istreambuf_iterator<char>()));
 ifs.close();
 return content;
}

void FileSystemAbstraction::SaveFile(const std::string &filePath, const std::string &content) {
 std::ofstream ostr(filePath);
 ostr << content;
 ostr.close();
}

void FileSystemAbstraction::CopyFile(const std::string &src, const std::string &dst) {
 std::experimental::filesystem::copy(src, dst);
}

void FileSystemAbstraction::CreateDirectory(const std::string &dirPath) {
 std::experimental::filesystem::create_directory(dirPath);
}
