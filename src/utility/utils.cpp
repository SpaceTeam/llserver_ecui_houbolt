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

    bool replaceRef(std::string& str, const std::string& from, const std::string& to) 
    {
        size_t start_pos = str.find(from);
        if(start_pos == std::string::npos)
            return false;
        str.replace(start_pos, from.length(), to);
        return true;
    }

    std::vector<std::string> split(const std::string& str, const std::string& delimiter)
    {
        std::vector<std::string> res;

        auto start = 0U;
        auto end = str.find(delimiter);
        while (end != std::string::npos)
        {
            res.push_back(str.substr(start, end - start));
            start = end + delimiter.length();
            end = str.find(delimiter, start);
        }

        res.push_back(str.substr(start, end));
        return res;
    }

    std::string merge(std::vector<std::string> strList, const std::string& delimiter)
    {
        std::string res = "";
        for (const auto &str : strList)
        {
            res += str + ":";
        }
        return res.substr(0, res.length() - delimiter.length());
    }

    uint64_t byteArrayToUInt64BigEndian(uint8_t *data)
    {
        uint64_t converted = 0;
        int i = 0;
        for (int shift=8*(sizeof(uint64_t)-1); shift > 0; shift-=8)
        {
            converted |= (uint64_t)data[i] << shift;
            i++;
        }
        converted |= (uint64_t)data[i];
        return converted;
    }

    void strToWCharPtr( const std::string& str, wchar_t *wCharStrOut)
    {
        std::wostringstream wstm ;
        const std::ctype<wchar_t>& ctfacet = 
                            std::use_facet< std::ctype<wchar_t> >( wstm.getloc() ) ;
        for( size_t i=0 ; i<str.size() ; ++i ) 
                wstm << ctfacet.widen( str[i] ) ;
        std::wstring wideStr = wstm.str() ;
        std::wcsncpy(wCharStrOut, wideStr.c_str(), wideStr.size()+1);
    }
}
