#ifndef CONFIGURATION_H_INCLUDE
#define CONFIGURATION_H_INCLUDE

#include <string>

/// Class for loading and managing configuration settings.
class configuration_manager
{

    int m_port;
    int m_server_threads;
    bool m_use_ssl;
    std::string m_listen_address;
    std::string m_log_verbosity;
    std::string m_log_sinks;
    std::string m_api_key;
    std::string m_vip;
    std::string m_ssl_key_file;
    std::string m_ssl_cert_chain_file;
    std::string m_ssl_dh_file;

    public:
        /// Constructor.
        configuration_manager();

        /// Returns configured port number.
        inline const int get_port() const { return m_port; };

        /// Returns the number of configured server threads.
        inline const int get_threads() const { return m_server_threads; };

        /// Returns configured listen address.
        inline const std::string& get_listen_address() const { return m_listen_address; };

        /// Returns the configured log verbosity.
        inline const std::string& get_log_verbosity() const { return m_log_verbosity; };

        /// Returns the configured log sinks.
        inline const std::string& get_log_sinks() const { return m_log_sinks; };

        /// Returns the configured api key clients need to authenticate against.
        inline const std::string& get_api_key() const { return m_api_key; };

        /// Returns the hostname:port of the keepalive VIP.
        inline const std::string& get_vip() const { return m_vip; };

        /// Returns the ssl private key.
        inline const std::string& get_ssl_key_file() const { return m_ssl_key_file; };

        /// Returns the ssl certificate chain file.
        inline const std::string& get_ssl_cert_chain_file() const { return m_ssl_cert_chain_file; };

        /// Returns the ssl dh512 file.
        inline const std::string& get_ssl_dh_file() const { return m_ssl_dh_file; };

        /// Returns true if ssl should be used.
        inline const bool get_ssl() const { return m_use_ssl; };

        /// Loads configuration from file.
        /// @param filename Path and filename to config file to load.
        /// @return Returns true if successful, false otherwise.
        bool open(const std::string& filename);

};

/// Global configuration manager instance.
extern configuration_manager config;

#endif
