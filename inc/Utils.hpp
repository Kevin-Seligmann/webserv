/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mvisca-g <mvisca-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 13:51:01 by irozhkov          #+#    #+#             */
/*   Updated: 2025/09/22 17:20:13 by mvisca-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

# pragma once

# include <algorithm>
# include <arpa/inet.h>
# include <climits>
# include <cstring>
# include <fstream>
# include <ifaddrs.h>
# include <iostream>
# include <map>
# include <netinet/in.h>
# include <sstream>
# include <stdexcept>
# include <string>
# include <sys/socket.h>
# include <unistd.h>
# include <vector>

// Forward declarations para evitar dependencias circulares
class Location;
struct ParsedServer;
# include "StringUtil.hpp"

# define CODE_ERR(str) (throw std::logic_error(std::string(str) + " at " + __FILE__ + ":" + wss::i_to_dec(__LINE__)))

enum ParsingMessageType {
    DEFAULT_SERVER,
    LOCAL_HOST,
    ASTERIKS_HOST
};

/* Check input file function */
int checkFile(const char* argv);

/* Download extencions for CGI*/
std::vector<std::string> loadCgiExtensions(const std::string& filename);

/* Function for getting real localhost */
std::string getLoopbackAddress();

/* Functions for printing */
void printLocationConfig(const Location& loc);
void printServerConfig(const ParsedServer& config);
void printParsingMessage(ParsingMessageType type);

/* Convert into a logs entry */
void OKlogsEntry(const std::string& title, const std::string& str);
void ERRORlogsEntry(const std::string& title, const std::string& str);

// String to size_t
size_t str_to_sizet(const std::string& str, size_t max_value);

std::string normalizePath(const std::string& base, const std::string& path);
std::string normalizeWebPath(const std::string& root, const std::string& request_path);

#endif
