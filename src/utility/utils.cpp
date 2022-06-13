//
// Created by Markus on 2019-09-28.
//

#include <chrono>


#include "utility/utils.h"


namespace utils
{


    int64_t toMicros(double val)
    {
        return (int64_t) (val * 1000000);
    }

    std::string loadFile(std::string filePath)
    {
        std::ifstream ifs(filePath);
        std::string content((std::istreambuf_iterator<char>(ifs)),
                            (std::istreambuf_iterator<char>()));
        ifs.close();
        return content;
    }

    void saveFile(std::string filePath, std::string content)
    {
        std::ofstream ostr(filePath);
        ostr << content;
        ostr.close();
    }

    bool keyExists(const nlohmann::json& j, const std::string& key)
    {
        return j.find(key) != j.end();
    }

    uint64_t getCurrentTimestamp()
    {
        const auto currTime = std::chrono::system_clock::now();
 
        return std::chrono::duration_cast<std::chrono::microseconds>(
                    currTime.time_since_epoch()).count();
    }

    void matrixMultiply(std::vector<std::vector<double>> &a, std::vector<std::vector<double>> &b, std::vector<std::vector<double>> &result)
    {
        for (size_t i = 0; i < a.size(); i++)
        {
            for (size_t j = 0; j < b[0].size(); j++)
            {
                for (size_t k = 0; k < a[0].size(); k++)
                {
                    result[i][j] = a[i][k] * b[k][j];
                }
            }
        }
    }

    std::string replace(const std::string& str, const std::string& from, const std::string& to) 
    {
        std::string newStr(str);
        size_t start_pos = str.find(from);
        if(start_pos == std::string::npos)
            return newStr;
        newStr.replace(start_pos, from.length(), to);
        return newStr;
    }

    bool replaceRef(std::string& str, const std::string& from, const std::string& to) {
        size_t start_pos = str.find(from);
        if(start_pos == std::string::npos)
            return false;
        str.replace(start_pos, from.length(), to);
        return true;
    }

    wchar_t * strToWCharPtr( const std::string& str )
    {
        std::wostringstream wstm ;
        const std::ctype<wchar_t>& ctfacet = 
                            std::use_facet< std::ctype<wchar_t> >( wstm.getloc() ) ;
        for( size_t i=0 ; i<str.size() ; ++i ) 
                wstm << ctfacet.widen( str[i] ) ;
        std::wstring wideStr = wstm.str() ;
        const wchar_t *constWideStr = wideStr.c_str();
        wchar_t * wideCharStr = const_cast< wchar_t * >( constWideStr );
        return wideCharStr;
    }
}
