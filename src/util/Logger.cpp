#include "Logger.hpp"
#include "HTTPRequest.hpp"

const std::string Logger::RED = "\033[1;91m";
const std::string Logger::GREEN = "\033[1;92m";
const std::string Logger::YELLOW = "\033[1;93m";
const std::string Logger::BLUE = "\033[1;94m";
const std::string Logger::RESET = "\033[0m";
const std::string Logger::ERR_PREFIX = RED + "Error. " + RESET;
const std::string Logger::WAR_PREFIX = YELLOW + "Warning. " + RESET;
const std::string Logger::INFO_PREFIX = BLUE + "Information. " + RESET;
const std::string Logger::DEBUG_PREFIX = GREEN + "Debug. " + RESET;

Logger::Logger(LogLevel log_level)
:_log_level(log_level),
_information_output(&std::cout),
_warning_output(&std::cout),
_error_output(&std::cerr)
{}

Logger::~Logger(){}

Logger & Logger::getInstance()
{
    static Logger instance(DEBUG); 
    return instance;
}

void Logger::info(std::string const & str) const
{
    if (_log_level == DEBUG || 
        _log_level == INFORMATION)
    {
        std::ostream * os = _information_output;
        _output_timestamp(os);
        *os << Logger::INFO_PREFIX << str << std::endl;
    }
}

void Logger::warning(std::string const & str) const
{
    if (_log_level == DEBUG || 
        _log_level == INFORMATION || 
        _log_level == WARNING)
    {
        std::ostream * os = _warning_output;
        _output_timestamp(os);
        *os << Logger::WAR_PREFIX << str << std::endl;
    }
}

void Logger::error(std::string const & str) const
{
    std::ostream * os = _error_output;
    _output_timestamp(os);
    *os << Logger::ERR_PREFIX << str << std::endl;
}

void Logger::setLogLevel(LogLevel log_level)
{
    _log_level = log_level;
}

void Logger::setOutput(LogLevel log_level, std::string const & str)
{
    std::ofstream os(str.c_str());
    if (os.fail())
        throw std::runtime_error("Can't open Logger output file");
    setOutput(log_level, os);
}

void Logger::setOutput(LogLevel log_level, std::ostream & os)
{
    switch (log_level)
    {
        case INFORMATION: _information_output = &os; return;
        case WARNING: _warning_output = &os ; return;
        case ERROR: _error_output = &os; return;
        default:
            throw std::runtime_error("Unknown log level type");
    }
}

void Logger::_output_timestamp(std::ostream * os) const
{
    time_t timestamp;
    time(&timestamp);
    char *str = ctime(&timestamp);
    str[std::strlen(str) - 1] = 0;
    *os << "[" << str << "] ";
}

std::ostream & Logger::operator<<(std::string const & str)
{
    _output_timestamp(_information_output);
    *_information_output << DEBUG_PREFIX << str;
    return *_information_output;
}

std::ostream & Logger::operator<<(HTTPRequest const & header)
{
    _output_timestamp(_information_output);
    *_information_output << DEBUG_PREFIX << header;
    return *_information_output;
}
