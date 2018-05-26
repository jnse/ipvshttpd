#ifndef LOGGING_H_INCLUDE
#define LOGGING_H_INCLUDE

#include <string>

/// Log levels.
enum LOG_LEVEL
{
    LOG_LEVEL_NONE=0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
};

/// Log sinks.
enum LOG_SINK
{
    LOG_SINK_NONE=0,
    LOG_SINK_STD=1,
    LOG_SINK_SYSLOG=2
};

/// Global log level.
extern int logging_level;

/// Global log sinks enabled.
extern int active_log_sinks;

/// Initializes logging.
void log_start();

/// Cleanup logging.
void log_stop();

/// Logs an error message.
/// @param message: Message to be logged.
void log_error(const std::string& message);

/// Logs an informational message.
/// @param message: Message to be logged.
void log_message(const std::string& message);

/// Logs a debug message.
/// @param message: Message to be logged.
void log_debug(const std::string& message);

#endif
