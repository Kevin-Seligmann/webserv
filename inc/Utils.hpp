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

# define RED        "\033[31m"
# define GREEN      "\033[32m"
# define YELLOW     "\033[33m"
# define DEFAULT    "\033[0m"

/* Check input file function */
int checkFile(const char* argv);

/* Functions for printing */
void printLocationConfig(const ParsedLocations& loc);
void printServerConfig(const ParsedServer& config);

#endif
