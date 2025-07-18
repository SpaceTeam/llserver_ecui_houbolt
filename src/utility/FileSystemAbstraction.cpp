#include "utility/FileSystemAbstraction.h"

#include <fstream>
#include <string>
#include <filesystem>

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
 std::filesystem::copy(src, dst);
}

void FileSystemAbstraction::CreateDirectory(const std::string &dirPath) {
 std::filesystem::create_directory(dirPath);
}
