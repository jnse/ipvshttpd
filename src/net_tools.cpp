#include <string.h>
#include <linux/socket.h>
#include <netdb.h>
#include "net_tools.h"

// Performs a dns lookup on hostname.
int host_to_addr(const char *name, struct in_addr *addr)                                                                             
{
    struct hostent *host;

    if ((host = gethostbyname(name)) != NULL) {
        if (host->h_addrtype != AF_INET ||
            host->h_length != sizeof(struct in_addr))
            return -1;
        /* warning: we just handle h_addr_list[0] here */
        memcpy(addr, host->h_addr_list[0], sizeof(struct in_addr));
        return 0;
    }
    return -1;
}

// Performs a reverse dns lookup on an IP.
std::string addr_to_host(int af, const void* addr)
{
    struct hostent* host = 0;
    if ((host = gethostbyaddr(static_cast<const char*>(addr),
        sizeof(struct in_addr), af)) != NULL) 
        return host->h_name;
    else
        return "";
}

// Performs a reverse dns lookup on an IP or returns the IP as string 
// if there is no reverse.
std::string addr_to_anyname(int af, const void* addr)
{
    static char buf[INET6_ADDRSTRLEN] = 0;
    if ((name = addr_to_host(af,addr)) != "") return name;
    inet_ntop(af,addr,buf,sizeof(buf));
    return buf;
}

