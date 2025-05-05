//
// Created by raffael on 04.05.25.
//

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>

#include "utility/Singleton.h"

class FileSystemAbstraction : public  Singleton<FileSystemAbstraction> {
 friend class Singleton;

public:
 virtual ~FileSystemAbstraction() = default;

 virtual std::string LoadFile(const std::string &filePath);

 virtual void SaveFile(const std::string &filePath, const std::string &content);

 virtual void CopyFile(const std::string &src, const std::string &dst);

 virtual void CreateDirectory(const std::string &dirPath);

};




#endif //FILESYSTEM_H
