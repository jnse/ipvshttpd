#include <string>
#include <sys/stat.h>
#include "file_tools.h"

// Checks if a file exists.
bool file_exists(const std::string& filename) 
{
    struct stat buffer;   
    return (stat(filename.c_str(),&buffer)==0); 
}
