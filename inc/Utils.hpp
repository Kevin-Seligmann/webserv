/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 13:51:01 by irozhkov          #+#    #+#             */
/*   Updated: 2025/06/12 14:14:47 by irozhkov         ###   ########.fr       */
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

# include "Parsed.hpp"

# define CODE_ERR(str) throw std::runtime_error(std::string(str) + " at " + __FILE__ + ":" + std::to_string(__LINE__))

/* Colores para debug y otros - COMENTADOS para evitar conflicto con Logger.hpp */
// # define RED        "\033[1;91m" 
// # define GREEN      "\033[1;92m"  
// # define YELLOW     "\033[1;93m"
// # define BLUE       "\033[1;94m"
// # define RESET      "\033[0m"

enum ParsingMessageType {
    DEFAULT_SERVER,
    IPV6_HOST,
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
