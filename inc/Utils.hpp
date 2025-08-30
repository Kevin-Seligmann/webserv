/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mvisca-g <mvisca-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 13:51:01 by irozhkov          #+#    #+#             */
/*   Updated: 2025/08/16 13:35:56 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

# pragma once

# include <arpa/inet.h>
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

/* Function for getting real localhost */
std::string getLoopbackAddress();

/* Functions for printing */
void printLocationConfig(const Location& loc);
void printServerConfig(const ParsedServer& config);
void printParsingMessage(ParsingMessageType type);

/* Convert into a logs entry */
void OKlogsEntry(const std::string& title, const std::string& str);
void ERRORlogsEntry(const std::string& title, const std::string& str);

#endif
