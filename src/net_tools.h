#ifndef NET_TOOLS_H_INCLUDE
#define NET_TOOLS_H_INCLUDE

#include <linux/socket.h>

/// Performs a dns lookup on hostname, stores result in addr pointer.
/// @param name Hostname to resolve.
/// @param addr Pointer to where the result is stored.
/// @return Returns -1 on failure, 0 on success.
int host_to_addr(const char *name, struct in_addr *addr);

/// Performs a reverse dns lookup on an IP.
/// @param af Address family.
/// @param Pointer to address structure for ip address.
/// @return Returns hostname of ip if successful, blank string ("") otherwise.
std::string addr_to_host(int af, const void* addr);

/// Performs a reverse dns lookup on an IP or returns the IP as string 
/// if there is no reverse.
/// @param af Address family.
/// @param Pointer to address structure for ip address.
/// @return Returns hostname of ip or ip as string on success.
/// @return Returns a blank string ("") on error.
std::string addr_to_anyname(int af, const void* addr);

#endif
