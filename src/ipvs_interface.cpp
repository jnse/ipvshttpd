
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <string>
#include <linux/socket.h>
#include <netdb.h>

#include "logging.h"
#include "net_tools.h"
#include "ipvs_interface.h"

// Constructor
ipvs_interface::ipvs_interface() : m_initialized(false)
{

}

// Destructor
ipvs_interface::~ipvs_interface()
{
    if (m_initialized == true)
    {
        ipvs_close();
    }
}

// Attempts to modprobe the ipvs kernel module.
int ipvs_interface::m_modprobe_ipvs()
{
    static const char* argv[] = { "/sbin/modprobe", "--", "ip_vs", NULL };
    int child;
    int status;
    int rc;
    if (!(child = fork())) {
        execv(const_cast<const char*>(argv[0]), 
            const_cast<char* const*>(argv));
        exit(1);
    }    

    rc = waitpid(child, &status, 0);

    if (rc == -1 || !WIFEXITED(status) || WEXITSTATUS(status)) {
        return 1;
    }    

    return 0;
}

// Populates proto from name.
int ipvs_interface::m_service_to_port(const char *name, unsigned short proto)
{           
    struct servent *service;            
    if (proto == IPPROTO_TCP
        && (service = getservbyname(name, "tcp")) != NULL)                                                                           
        return ntohs((unsigned short) service->s_port);
    else if (proto == IPPROTO_UDP
         && (service = getservbyname(name, "udp")) != NULL)
        return ntohs((unsigned short) service->s_port);
    else if (proto == IPPROTO_SCTP
         && (service = getservbyname(name, "sctp")) != NULL)
        return ntohs((unsigned short) service->s_port);
    else
        return -1; 
}

// strtol() with min/max.
int ipvs_interface::m_string_to_number(const char* s, int min, int max)
{           
    long number;
    char *end;            
    errno = 0;
    number = strtol(s, &end, 10);
    if (*end == '\0' && end != s) {
        // We parsed a number, let's see if we want this.
        if (errno != ERANGE && min <= number && number <= max)
            return number;
    }       
    return -1;
}

// Check if string is digit.
int ipvs_interface::m_str_is_digit(const char* str)
{
    size_t offset;
    size_t top;
    top = strlen(str);
    for (offset=0; offset<top; offset++) {
        if (!isdigit((int)*(str+offset))) {
            break; 
        }
    }   
    return (offset<top)?0:1;
}

// Get IP address and port from argument.
int ipvs_interface::m_parse_service(char* buf, ipvs_service_t *svc)
{
    char *portp = NULL;
    long portn;
    int result=SERVICE_NONE;
    struct in_addr inaddr;
    struct in6_addr inaddr6;
    if (buf == NULL || m_str_is_digit(buf))
        return SERVICE_NONE;
    if (buf[0] == '[') {
        buf++;
        portp = strchr(buf, ']');
        if (portp == NULL)
            return SERVICE_NONE;
        *portp = '\0';
        portp++;
        if (*portp == ':')
            *portp = '\0';
        else
            return SERVICE_NONE;
    }
    if (inet_pton(AF_INET6, buf, &inaddr6) > 0) {
        svc->addr.in6 = inaddr6;
        svc->af = AF_INET6;
        svc->netmask = 128;
    } else {
        portp = strrchr(buf, ':');
        if (portp != NULL)
            *portp = '\0';

        if (inet_aton(buf, &inaddr) != 0) {
            svc->addr.ip = inaddr.s_addr;
            svc->af = AF_INET;
        } else if (host_to_addr(buf, &inaddr) != -1) {
            svc->addr.ip = inaddr.s_addr;
            svc->af = AF_INET;
        } else
            return SERVICE_NONE;
    }

    result |= SERVICE_ADDR;

    if (portp != NULL) {
        result |= SERVICE_PORT;

        if ((portn = m_string_to_number(portp+1, 0, 65535)) != -1)
            svc->port = htons(portn);
        else if ((portn = m_service_to_port(portp+1, svc->protocol)) != -1)
            svc->port = htons(portn);
        else
            return SERVICE_NONE;
    }

    return result;
}

// Initializes the ipvs interface.
bool ipvs_interface::init()
{
    if (ipvs_init())
    {
        if (m_modprobe_ipvs() || ipvs_init())
        {
            log_error(std::string("Can't initialize ipvs: ")
                +ipvs_strerror(errno));
            return false;
        }
    }
    m_initialized = true;
    return true;
}

// Get ipvs service object for a host.
bool ipvs_interface::set_weight(const std::string& vip, const std::string& server, int weight)
{
    // set command defaults
    struct ipvs_command_entry ce;
    int parse = 0;
    int result = 0;
    memset(&ce, 0, sizeof(struct ipvs_command_entry));
    ce.dest.weight = weight;
    ce.dest.conn_flags = IP_VS_CONN_F_DROUTE;
    ce.svc.netmask = ((u_int32_t) 0xffffffff);
    ce.svc.protocol = IPPROTO_TCP;
    // parse vip
    std::string vip_copy = vip;
    parse = m_parse_service(const_cast<char*>(vip_copy.c_str()),&ce.svc);
    if (!(parse & SERVICE_ADDR))
    {
        log_error("Illegal VIP address[:port] specified.");
        return false;
    }
    // parse server
    ipvs_service_t t_dest = ce.svc;
    parse = m_parse_service(const_cast<char*>(server.c_str()),&t_dest);
    ce.dest.af = t_dest.af;
    ce.dest.addr = t_dest.addr;
    ce.dest.port = t_dest.port;
    ce.dest.weight = weight;
    // copy vport to dport if not specified.
    if (parse == 1) ce.dest.port = ce.svc.port;
    if (!(parse & SERVICE_ADDR))
    {
        log_error("Illegal server address[:port] specified.");
        return false;
    }
    // copy vport to dport if not specified.
    if (parse == 1) ce.dest.port = ce.svc.port;
    // Make sure that the port zero service is persistent.
    if (!ce.svc.fwmark && !ce.svc.port &&
        !(ce.svc.flags & IP_VS_SVC_F_PERSISTENT))
    {
        log_error("Zero port specified for non-persistent service");
        return false;
    }
    if (ce.svc.flags & IP_VS_SVC_F_ONEPACKET &&
        !ce.svc.fwmark && ce.svc.protocol != IPPROTO_UDP)
    {
        log_error("One-Packet Scheduling is only for UDP virtual services");
        return false;
    }
    // Set the default scheduling algorithm if not specified.
    if (strlen(ce.svc.sched_name) == 0)
    {
        strcpy(ce.svc.sched_name, "wlc");
    }
    //result = ipvs_update_service(&ce.svc);
    result = ipvs_update_dest(&ce.svc,&ce.dest);
    return (result == 0);;
}


