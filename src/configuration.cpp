#include <stdlib.h>
#include <string>
#include <fstream>
#include "string_tools.h"
#include "logging.h"
#include "configuration.h"

// Global configuration manager instance.
configuration_manager config;

// Constructor.
configuration_manager::configuration_manager()
    : m_port(8080), m_listen_address("0.0.0.0"),
      m_server_threads(2), m_use_ssl(false)
{

}

// Loads configuration from file.
bool configuration_manager::open(const std::string& filename)
{
    log_message("Reading configuration...");
    std::string line;
    std::ifstream fin(filename.c_str());
    if (!fin.is_open())
    {
        log_error("Could not open configuration file: "+filename);
        return false;
    }
    int line_nr = 0;
    bool valid = true;
    while(std::getline(fin,line))
    {
        line_nr++;
        line = str_trim(line);
        // Immediately skip blank lines.
        if (line == "") continue;
        // Skip comments.
        if (str_starts_with("#",line)==true) continue;
        // Split line in key-value pair.
        str_vector kvp = str_split(line,":",2);
        if (kvp.size() != 2)
        {
            log_error(std::string("Invalid configuration in ")
                + filename + std::string(" line ")
                + str_from_number(line_nr));
            valid = false;
            break;
        }
        std::string key = str_to_lower(kvp[0]);
        std::string value = str_trim(kvp[1]);
        if (key == "port")
        {
            m_port = atoi(value.c_str());
        }
        else if (key == "listen_address")
        {
            m_listen_address = value;
        }
        else if (key == "log_verbosity")
        {
            m_log_verbosity = value;
        }
        else if (key == "log_sinks")
        {
            m_log_sinks = value;
        }
        else if (key == "threads")
        {
            m_server_threads = atoi(value.c_str());
        }
        else if (key == "api_key")
        {
            m_api_key = value;
        }
        else if (key == "vip")
        {
            m_vip = value;
        }
        else if (key == "use_ssl")
        {
            std::string lval = str_to_lower(value);
            if ((lval == "yes") or (lval == "true"))
            {
                m_use_ssl = true;
            }
        }
        else if (key == "ssl_key_file")
        {
            m_ssl_key_file = value;
        }
        else if (key == "ssl_cert_chain_file")
        {
            m_ssl_cert_chain_file = value;
        }
        else if (key == "ssl_dh_file")
        {
            m_ssl_dh_file = value; 
        }
        else
        {
            log_error(std::string("Invalid configuration in ")
                + filename + std::string(" line ")
                + str_from_number(line_nr));
            valid = false;
            break;
        }
    }
    fin.close();
    return valid;
}


