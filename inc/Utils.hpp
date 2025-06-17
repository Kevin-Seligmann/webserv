/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/30 13:51:01 by irozhkov          #+#    #+#             */
/*   Updated: 2025/05/31 12:44:00 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

# pragma once

# include <fstream>
# include <iostream>
# include <map>
# include <sstream>
# include <stdexcept>
# include <string>
# include <vector>

# include "Parsed.hpp"

// Forward declaration to avoid circular dependency
class VirtualServersManager;

/* Colores para debug y otros */
# define RED        "\033[1;91m" 
# define GREEN      "\033[1;92m"  
# define YELLOW     "\033[1;93m"
# define BLUE       "\033[1;94m"
# define RESET      "\033[0m"

/* Check input file function */
int checkFile(const char* argv);

/* Functions for printing */
void printLocationConfig(const Locations& loc);
void printServerConfig(const ParsedServer& config);

/* Server initialization functions */
// implemented in serversInit.cpp
bool serversInit(VirtualServersManager& sm, const ParsedServers& ps);

/* Server startup functions */
// implemented in serversStart.cpp
bool event_loop(VirtualServersManager &sm);

bool serversStart(VirtualServersManager& sm);

/* Convert into a logs entry */
void OKlogsEntry(const std::string& title, const std::string& str);
void ERRORlogsEntry(const std::string& title, const std::string& str);

#endif
