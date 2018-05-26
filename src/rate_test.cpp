/**
 * See how fast we can update weight in keepalived
 */

#include <iostream>
#include <stdlib.h>
#include "ipvs_interface.h"

/// Show program usage to end-user.
/// @param progname Executable name passed from argv[0]
void usage(const std::string& progname)
{
    std::cout << "Usage: " << progname << " [vip] [server name] [weight]" << std::endl;
}

/// Entry point.
int main(int argc, char* argv[])
{
    // Check arguments.
    if (argc != 4)
    {
        usage(argv[0]);
        return 1;
    }
    // Initialize ipvs.
    ipvs_interface ipvs;
    if (ipvs.init() != true) return 1;
    // Set weight.
    if (ipvs.set_weight(argv[1],argv[2],atoi(argv[3])) != true)
    {
        return 1;
    }
    return 0;
}
