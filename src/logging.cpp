#include <string>
#include <iostream>
#include <syslog.h>
#include "logging.h"

// Global log level.
int logging_level=0;

// Global log sinks enabled.
int active_log_sinks=0;

// Initialize logging.
void log_start()
{
    if ((active_log_sinks & LOG_SINK_SYSLOG) == LOG_SINK_SYSLOG) 
    {
        openlog("ipvshttpd",LOG_CONS|LOG_PID|LOG_NDELAY,LOG_LOCAL1);
    }
}

// Cleanup logging.
void log_stop()
{
    if ((active_log_sinks & LOG_SINK_SYSLOG) == LOG_SINK_SYSLOG) 
    {
        closelog();
    }
}

// Logs an error message.
void log_error(const std::string& message)
{
    if (logging_level < LOG_LEVEL_ERROR) return;
    if (active_log_sinks == LOG_SINK_NONE) return;
    if ((active_log_sinks & LOG_SINK_STD) == LOG_SINK_STD)
    {
        std::cerr << message << std::endl;
    }
    if ((active_log_sinks && LOG_SINK_SYSLOG) == LOG_SINK_SYSLOG)
    {
        syslog(LOG_WARNING,message.c_str());
    }
}

// Logs an informational message.
void log_message(const std::string& message)
{
    if (logging_level < LOG_LEVEL_INFO) return;
    if (active_log_sinks == LOG_SINK_NONE) return;
    if ((active_log_sinks & LOG_SINK_STD) == LOG_SINK_STD)
    {
        std::cout << message << std::endl;
    }
    if ((active_log_sinks & LOG_SINK_SYSLOG) == LOG_SINK_SYSLOG)
    {
        syslog(LOG_INFO,message.c_str());
    }
}

// Logs a debug message.
void log_debug(const std::string& message)
{
    if (logging_level < LOG_LEVEL_DEBUG) return;
    if (active_log_sinks == LOG_SINK_NONE) return;
    if ((active_log_sinks & LOG_SINK_STD) == LOG_SINK_STD)
    {
        std::cout << message << std::endl;
    }
    if ((active_log_sinks & LOG_SINK_SYSLOG) == LOG_SINK_SYSLOG)
    {
        syslog(LOG_DEBUG,message.c_str());
    }
}

