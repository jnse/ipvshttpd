#include "string.h"
#include <string>
#include <sstream>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include "string_tools.h"

// Converts integer to string.
std::string str_from_number(int num)
{
    std::stringstream result;
    result << num;
    return result.str();
}

// Converts size to string.
std::string str_from_number(size_t num)
{
    std::stringstream result;
    result << num;
    return result.str();
}

// Trims whitespace from left of a string.
std::string str_ltrim(const std::string& str)
{
    std::string s = str;
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
        std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// Trims whitespace from the right of a string.
std::string str_rtrim(const std::string& str)
{
    std::string s = str;
    s.erase(std::find_if(s.rbegin(), s.rend(), 
        std::not1(std::ptr_fun<int, int>(std::isspace))).base(), 
        s.end());
    return s;
}

// Trim whitespace from beginning and end of a string.
std::string str_trim(const std::string& str) 
{
    return str_ltrim(str_rtrim(str));
}

// Splits a string using another string as delimiter.
str_vector str_split(const std::string& str, const std::string& delim, int max_elem)
{
    size_t pos=0;
    std::string s = str;
    str_vector result;
    int tokens = 1;
    while (((pos = s.find(delim)) != std::string::npos)
        and ((max_elem < 0) or (tokens != max_elem)))
    {
        result.push_back(s.substr(0,pos));
        s.erase(0, pos + delim.length());
        tokens++;
    }
    if (max_elem != 0) result.push_back(s);
    return result;
}

// Converts a string to lowercase.
std::string str_to_lower(const std::string& str)
{
    std::string copy = str;
    std::transform(copy.begin(),copy.end(),copy.begin(),::tolower);
    return copy;
}

// Returns true if string haystack starts with needle.
bool str_starts_with(const std::string& needle, const std::string& haystack)
{
    return (strncmp(haystack.c_str(),needle.c_str(),needle.size()) == 0);
}

