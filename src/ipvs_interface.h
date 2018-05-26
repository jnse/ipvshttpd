/**
 * C++ wrapper around libipvs
 * 
 * Tries to hide some of the uglies of dealing with the ipvs library.
 *
 */
#ifndef IPVS_INTERFACE_H_INCLUDE
#define IPVS_INTERFACE_H_INCLUDE

extern "C" 
{
    #include <libipvs/libipvs.h>
}


#define SERVICE_NONE 0x0000
#define SERVICE_ADDR 0x0001
#define SERVICE_PORT 0x0002

struct ipvs_command_entry 
{
    ipvs_service_t      svc;
    ipvs_dest_t         dest;
    ipvs_timeout_t      timeout;
    ipvs_daemon_t       daemon;
};

/**
 * Interface to the ipvs C library.
 */
class ipvs_interface
{

    /// Gets set to true if ipvs was successfully initialized.
    bool m_initialized;
    /// Attempts to modprobe the ipvs kernel module.
    /// @return Returns 1 on error, 0 if successful.
    static int m_modprobe_ipvs();
    /// Get IP address and port from argument.
    static int m_parse_service(char* bug, ipvs_service_t *svc);
    /// Check if string is digit.
    static int m_str_is_digit(const char* str);
    /// strtol() with min/max.
    static int m_string_to_number(const char* s, int min, int max);
    /// populates proto from name.
    static int m_service_to_port(const char *name, unsigned short proto);
    public:

        /// Constructor
        ipvs_interface();

        /// Destructor
        ~ipvs_interface();

        /// Initializes the ipvs interface.
        /// @return Returns true if successful, false otherwise.
        bool init();

        /// Sets weight for an IPVS service..
        /// @param vip Virtual IP host and port.
        /// @param server Server host and port.
        /// @param weight Weight to set for server.
        /// @return Returns true if successful, false otherwise.
        bool set_weight(const std::string& vip, const std::string& server, int weight);
};

#endif
