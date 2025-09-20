/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mvisca-g <mvisca-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 13:58:20 by irozhkov          #+#    #+#             */
/*   Updated: 2025/09/20 14:01:57 by mvisca-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils.hpp"
#include "Location.hpp"
#include "Parsed.hpp"

// Temporary color macros to fix compilation
#define RED        "\033[1;91m"
#define GREEN      "\033[1;92m"
#define YELLOW     "\033[1;93m"
#define BLUE       "\033[1;94m"
#define RESET      "\033[0m"

int checkFile(const char* argv)
{
    std::ifstream infile(argv);

    if (!infile.is_open())
    {
        std::cerr << std::endl << RED <<
        "ERROR: file does not exist or can not be opened." <<
        RESET << std::endl << std::endl;
        return (-1);
    }

    infile.seekg(0, std::ios_base::end);
    if (infile.tellg() == 0)
    {
        std::cerr << std::endl << RED <<
        "ERROR: file is empty." <<
        RESET << std::endl << std::endl;
        return (-1);
    }
    infile.seekg(0, std::ios_base::beg);

    if (infile.fail())
    {
        std::cerr << std::endl << RED <<
        "ERROR: file stream error detected." <<
        RESET << std::endl << std::endl;
        return (-1);
    }

    if (infile.is_open()) { infile.close(); }

    return (0);
}

std::string autoindexToString(AutoIndexState state)
{
    switch (state)
    {
        case AINDX_DEF_OFF:  return "DEFAULT_OFF";
        case AINDX_DEF_ON:   return "DEFAULT_ON";
        case AINDX_SERV_OFF: return "SERVER_OFF";
        case AINDX_SERV_ON:  return "SERVER_ON";
        case AINDX_LOC_OFF:  return "LOCATION_OFF";
        case AINDX_LOC_ON:   return "LOCATION_ON";
        default:             return "UNKNOWN";
    }
}

std::vector<std::string> loadCgiExtensions(const std::string& filename) {
	std::vector<std::string> extensions;
	std::ifstream file(filename.c_str());
	std::string line;

	while (std::getline(file, line))
	{
		if (!line.empty())
			extensions.push_back(line);
	}

	return (extensions);
}

void printLocationConfig(const Location& loc)
{
	std::cout << YELLOW << "    Autoindex: " << RESET << autoindexToString(loc.getAutoindex()) << std::endl;

	std::cout << YELLOW << "    Root: " << RESET << loc.getRoot() << std::endl;
	std::cout << YELLOW << "    Alias: " << RESET << loc.getAlias() << std::endl;
	std::vector<std::string> methods = loc.getMethods();
	std::cout << YELLOW << "    Allowed Methods:" << RESET << std::endl;
    for (size_t i = 0; i < methods.size(); ++i)
        std::cout << "      - " << methods[i] << std::endl;


	// TODO: Implement when Location class has proper getters
//	(void)loc; // Suppress unused parameter warning
//	std::cout << "    Location configuration (not implemented yet)" << std::endl;
}

void printServerConfig(const ParsedServer& config)
{

	std::cout << GREEN << "\n=== ServerConfig ===" << RESET << std::endl;

	std::cout << YELLOW << "Listen Directives:" << RESET << std::endl;
	for (size_t i = 0; i < config.listens.size(); ++i)
		std::cout << "  - " << config.listens[i].to_string() << std::endl;

	std::cout << YELLOW << "Server Names:" << RESET << std::endl;
	for (size_t i = 0; i < config.server_names.size(); ++i)
		std::cout << "  - " << config.server_names[i] << std::endl;

	std::cout << YELLOW << "Root: " << RESET << config.root << std::endl;
	
	std::cout << YELLOW << "Error Pages:" << RESET << std::endl;
	for (std::map<int, std::string>::const_iterator it = config.error_pages.begin();
		 it != config.error_pages.end(); ++it)
		std::cout << "  " << it->first << " => " << it->second << std::endl;

	std::cout << YELLOW << "Allowed Methods:" << RESET << std::endl;
	for (size_t i = 0; i < config.allow_methods.size(); ++i)
		std::cout << "  - " << config.allow_methods[i] << std::endl;

//	std::cout << YELLOW << "Autoindex: " << RESET << (config.autoindex ? "true" : "false") << std::endl;

	std::cout << YELLOW << "Autoindex: " << RESET << autoindexToString(config.autoindex) << std::endl;


	std::cout << YELLOW << "Client Max Body Size: " << RESET << config.client_max_body_size << std::endl;

	std::cout << YELLOW << "Locations:" << RESET << std::endl;
	for (std::map<std::string, Location>::const_iterator it = config.locations.begin();
		 it != config.locations.end(); ++it)
	{
		std::cout << BLUE <<"  Location block for: " << it->first << RESET << std::endl;
		printLocationConfig(it->second);
	}

	std::cout << std::endl;
}

void printParsingMessage(ParsingMessageType type)
{
	switch (type)
	{
		case DEFAULT_SERVER:
			std::cout << YELLOW << "INFO: " << RESET <<
			"According to the subject requirements, if two or more servers are configured " <<
			"with the same host:port, the first one in order is considered the primary. The " <<
			YELLOW << "[default_server]" << RESET << " directive is " << YELLOW <<
			"ignored" << RESET << "." << std::endl;
			break;
		case LOCAL_HOST:
			std::cout << YELLOW << "INFO: " << RESET <<
			"Hosts like " << YELLOW << "localhost" << RESET << " will be " << YELLOW <<
			"replaced with the real localhost. " << RESET <<
			"According to subject, if this replace will affect same host:port of other server, " <<
			YELLOW << "the first one in order is considered the primary" << RESET << "." << std::endl;
			break;
		case ASTERIKS_HOST:
			std::cout << YELLOW << "INFO: " << RESET <<
			"Hosts like " << YELLOW << "*" << RESET << " will be " << YELLOW <<
			"replaced with default host. " << RESET <<
			"According to subject, if this replace will affect same host:port of other server, " <<
			YELLOW << "the first one in order is considered the primary" << RESET << "." << std::endl;
			break;
		default:
			std::cout << YELLOW << "INFO: " << RESET << "Unknown parsing state." << std::endl;
			break;
	}
}

std::string getLoopbackAddress()
{
	struct ifaddrs* ifaddr;
	if (getifaddrs(&ifaddr) == -1)
		return ("0.0.0.0");

	std::string loopbackIP = "127.0.0.1";
	for (struct ifaddrs* ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
	{
		if (!ifa->ifa_addr)
			continue;

		if (ifa->ifa_addr->sa_family == AF_INET && std::string(ifa->ifa_name) == "lo")
		{
			char addrBuf[INET_ADDRSTRLEN];
			void* addrPtr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
			inet_ntop(AF_INET, addrPtr, addrBuf, INET_ADDRSTRLEN);
			loopbackIP = std::string(addrBuf);
			break;
		}
	}

	freeifaddrs(ifaddr);
	return (loopbackIP);
}

/* Logs Utility */

void OKlogsEntry(const std::string& title, const std::string& str) {
	std::cout << BLUE << title << RESET << str << std::endl;
}

void ERRORlogsEntry(const std::string& title, const std::string& str) {
	std::cout << RED << title << RESET << str << std::endl;
}

size_t str_to_sizet(const std::string& str, size_t max_value) {
    if (str.empty()) {
        return 0;
    }

	char* endptr = NULL;
	unsigned long base = strtoul(str.c_str(), &endptr, 10);

	if (base == 0 || base == ULONG_MAX) {
		return 0;
	}

	if (*endptr != '\0') {
		switch (std::toupper(*endptr)) {
			case 'K':
				base *= 1024UL;
				break;
			case 'M':
				base *= 1024UL * 1024UL;
				break;
			case 'G':
				base *= 1024UL * 1024UL * 1024UL;
				break;
		default:
			// Si el sufijono es valido se usa el valor inicial bruto
			break;
		}
	}

	if (base > max_value) {
		return max_value;
	}

	return static_cast<size_t>(base);    
}

std::string normalizePath(const std::string& base, const std::string& path) {
	std::string result;
	
	if (!base.empty() && base[base.length() - 1] == '/') {
		result = base.substr(0, base.length() - 1);
	} else {
		result = base;
	}

	if (path.empty() || path[0] != '/') {
		result += "/" + path;
	} else {
		result += path;
	}

	return result;
}
