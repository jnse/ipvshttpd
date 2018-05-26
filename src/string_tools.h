#ifndef STRING_TOOLS_H_INCLUDE
#define STRING_TOOLS_H_INCLUDE

#include <string>
#include <vector>

/// Convenience typedef for a vector of strings.
typedef std::vector<std::string> str_vector;

/// Converts integer to string.
/// @param num Number to convert.
/// @return Returns number as string.
std::string str_from_number(int num);

/// Converts size to string.
/// @param num Number to convert.
/// @return Returns number as string.
std::string str_from_number(size_t num);

/// Trims whitespace from the left of a string.
/// @param str String to trim.
/// @returns A copy of str trimmed for whitespace from the left.
std::string str_ltrim(const std::string& str);

/// Trims whitespace from the right of a string.
/// @param str String to trim.
/// @return A copy of str trimmed for whitespace from the right.
std::string str_rtrim(const std::string& str);

/// Trims whitespace from beginning and end of a string.
/// @param str String to trim.
/// @return A copy of str trimmed from both ends of the string.
std::string str_trim(const std::string& str);

/// Splits a string using another string as delimiter.
/// @param str : String to split.
/// @param delim : Delimiter to use.
/// @param max_elem : (optional) Split string up to maximum max_elem times.
/// @return str_vector : Returns a vector of strings containing string str 
///                      substrings split up by delim.
str_vector str_split(const std::string& str, const std::string& delim, 
    int max_elem=-1);

/// Converts a string to lowercase.
/// @param str : String to transform.
/// @return Returns str transformed to lower case.
std::string str_to_lower(const std::string& str);

/// Check if a string starts with another substring.
/// @param haystack : String to check.
/// @param needle : Substring to look for in haystack string.
/// @return Returns true if haystack starts with needle. False otherwise.
bool str_starts_with(const std::string& needle, const std::string& haystack);

#endif
