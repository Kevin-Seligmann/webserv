#pragma once

#include <stdexcept>
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include <cstring>



struct HTTPRequest;

enum LogLevel 
{
    DEBUG, INFORMATION, WARNING, ERROR
};

class Logger
{
public:
    static const std::string RED;
    static const std::string GREEN;
    static const std::string YELLOW;
    static const std::string BLUE;
    static const std::string RESET;

    Logger(LogLevel log_level);
    ~Logger();

    static Logger & getInstance();

    void info(std::string const & str) const;
    void error(std::string const & str) const;
    void warning(std::string const & str) const;

    void setLogLevel(LogLevel log_level);

    void setOutput(LogLevel log_level, std::string const & str);
    void setOutput(LogLevel log_level, std::ostream & out);


    std::ostream & operator<<(std::string const & str);
    std::ostream & operator<<(HTTPRequest const & header);

private:
    static const std::string ERR_PREFIX;
    static const std::string WAR_PREFIX;
    static const std::string INFO_PREFIX;
    static const std::string DEBUG_PREFIX;

    LogLevel _log_level; 
    std::ostream * _information_output;
    std::ostream * _warning_output;
    std::ostream * _error_output;
    std::ofstream   _dump;

    void _output_timestamp(std::ostream * os) const;

    Logger(Logger &);
    Logger & operator=(Logger &);
};

