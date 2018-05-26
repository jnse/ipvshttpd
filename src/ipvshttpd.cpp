#include <stdexcept>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fstream>
#include <iostream>
#include <string>
#include <pwd.h>
#include <grp.h>
#include "logging.h"
#include "string_tools.h"
#include "file_tools.h"
#include "configuration.h"
#include "http_server.h"

/// Show program usage.
void usage(const std::string& program_name)
{
    std::cout << program_name;
    std::cout << "\t";
    std::cout << "[-c <config filename>] ";
    std::cout << "[-f] ";
    std::cout << "[-l (DEBUG|INFO|ERROR|NONE)] ";
    std::cout << std::endl << "\t\t";
    std::cout << "[-L (STDOUT|SYSLOG|NONE)] ";
    std::cout << "[-p <pidfile filename>] ";
    std::cout << std::endl << std::endl;
    std::cout << "-c <file>\tConfiguration file (followed by path to config";
    std::cout << "file).";
    std::cout << std::endl;
    std::cout << "-f\t\tKeep in foreground (prevents daemonizing, dropping";
    std::cout << "privileges).";
    std::cout << std::endl;
    std::cout << "-l <level>\tLogging level (one of DEBUG, INFO, ERROR or NONE)";
    std::cout << std::endl;
    std::cout << "-L <sink>\tLog sinks (comma-separated list of log sinks ";
    std::cout << "(stdout, syslog, or none)";
    std::cout << std::endl;
    std::cout << "-p <file>\tWrites out a PIDfile with given filename.";
    std::cout << std::endl;
}

// Parses configured log sinks.
bool parse_log_sink(const std::string& selected_log_sink)
{
    if ((selected_log_sink != "none") and (selected_log_sink != "syslog")
        and (selected_log_sink != "stdout"))
    {
        log_error("Invalid log sink: "+selected_log_sink);
        return false;
    }
    if (selected_log_sink == "none")
    {
        active_log_sinks |= LOG_SINK_NONE;
    }
    if (selected_log_sink == "stdout")
    {
        active_log_sinks |= LOG_SINK_STD;
    }
    if (selected_log_sink == "syslog")
    {
        active_log_sinks |= LOG_SINK_SYSLOG;
    }
    return true;
}

// Parses configured log level.
bool parse_log_level(const std::string& log_level_choice)
{
    if (log_level_choice == "none")
    {
        logging_level = LOG_LEVEL_NONE;
    }
    else if (log_level_choice == "error")
    {
        logging_level = LOG_LEVEL_ERROR;
    }
    else if (log_level_choice == "info")
    {
        logging_level = LOG_LEVEL_INFO;
    }
    else if (log_level_choice == "debug")
    {
        logging_level = LOG_LEVEL_DEBUG;
    }
    else
    {
        log_error("Unknown log level: "+log_level_choice);
        return false;
    }
    return true;
}

/// Entry point.
int main(int argc, char* argv[])
{
    // Defaults.
    std::string config_file = "/etc/sysconfig/ipvshttpd.conf";
    std::string pid_file = "";
    logging_level = LOG_LEVEL_ERROR;
    active_log_sinks = LOG_SINK_STD;
    bool config_set = false;
    bool log_level_set = false;
    bool log_sinks_set = false;
    bool daemonize = true;
    // Parse arguments.
    for (int argn = 1; argn != argc; ++argn)
    {
        std::string curr_opt = argv[argn];
        std::string next_opt = "";
        if (argn+1 < argc) next_opt=argv[argn+1];
        if (curr_opt == "-f")
        {
            daemonize = false;
        }
        else if (curr_opt == "-l")
        {
            if ((next_opt == "") or (str_starts_with("-",next_opt)))
            {
                log_error("Missing argument to -l option.");
                usage(argv[0]);
                return 1;
            }
            std::string log_level_choice = str_to_lower(next_opt);
            if (parse_log_level(log_level_choice) != true)
            {
                return 1;
            }
            log_level_set = true;
            argn++;
        }
        else if (curr_opt== "-L")
        {
            if ((next_opt == "") or (str_starts_with("-",next_opt)))
            {
                log_error("Missing argument(s) to -L option.");
                usage(argv[0]);
                return 1;
            }
            str_vector selected_log_sinks = str_split(next_opt,",");
            for (str_vector::iterator it = selected_log_sinks.begin(); 
                it != selected_log_sinks.end(); ++it)
            {
                std::string selected_log_sink = str_to_lower(*it);
                if (parse_log_sink(selected_log_sink) != true)
                {
                    return 1;
                }
            }
            log_sinks_set = true;
            argn++;
        }
        else if (curr_opt == "-c")
        {
            if ((next_opt == "") or (str_starts_with("-",next_opt)))
            {
                log_error("Missing argument to -c option.");
                usage(argv[0]);
                return 1;
            }
            argn++;
            config_file = argv[argn];
            config_set = true;
        }
        else if (curr_opt == "-p")
        {
            if ((next_opt == "") or (str_starts_with("-",next_opt)))
            {
                log_error("Missing argument to -p option.");
                usage(argv[0]);
                return 1;
            }
            argn++;
            pid_file = argv[argn];
        }
        else
        {
            log_error(std::string("Unknown option: ")+argv[argn]);
            usage(argv[0]);
            return 1;
        }
    }
    // Load configuration file if one was specified at the commandline
    // or if the default config file exists.
    if ((config_set == true) or (file_exists(config_file)))
    {
        if (config.open(config_file) != true)
        {
            log_error("Could not load configuration file.");
            return 1;
        }
        // Only set log level and sinks from config file if not
        // overridden on command line.
        if (log_level_set != true)
        {
            std::string configured_log_level = config.get_log_verbosity();
            if (configured_log_level != "")
            {
                if (parse_log_level(configured_log_level) != true)
                {
                    return 1;
                }
            }
        }
        if (log_sinks_set != true)
        {
            std::string configured_log_sinks = config.get_log_sinks();
            if (configured_log_sinks != "")
            {
                str_vector selected_log_sinks = str_split(configured_log_sinks,",");
                for (str_vector::iterator it = selected_log_sinks.begin();
                    it != selected_log_sinks.end(); ++it)
                {
                    std::string selected_log_sink = str_to_lower(*it);
                    if (parse_log_sink(selected_log_sink) != true)
                    {
                        return 1;
                    }
                }
            }
        }
    }
    log_start();
    log_message("ipvshttpd starting...");
    http_server server(
        config.get_listen_address(),
        config.get_port(),
        config.get_threads()
    );
    // Daemonize if desired.
    pid_t pid = 0;
    if (daemonize == true)
    {
        pid = fork();
        if (pid < 0)
        {
            log_error("Could not fork.");
            log_stop();
            return 1;
        }
        if (pid > 0)
        {
            // Parent, just exit.
            return 0;
        }
        umask(0);
    }
    // Write out a PID file if desired.
    if (pid_file != "")
    {
        pid_t pid = getpid();
        std::ofstream fout(pid_file.c_str());
        if (!fout.is_open())
        {
            log_error("Could not write to PID file: "+pid_file);
            return 1;
        }
        fout << pid << std::endl;
        fout.close();
    }
    server.start();
    server.stop();
    log_stop();
    // Delete pid file if we wrote one out.
    if (pid_file != "")
    {
        if (remove(pid_file.c_str()) != 0)
        {
            log_error("Could not clean up PID file: "+pid_file);
            return 1;
        }
    }
    return 0;
}
