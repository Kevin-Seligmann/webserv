/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIInterpreter.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/15 10:51:09 by irozhkov          #+#    #+#             */
/*   Updated: 2025/09/15 13:40:56 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#ifndef CGIINTERPTETER_HPP
# define CGIINTERPTETER_HPP

# pragma once

# include <string>
# include <vector>
# include <utility>
# include <fstream>
# include <sstream>
# include <iostream>
# include "Logger.hpp"
# include "StringUtil.hpp"

struct CGIInterpreter;

typedef std::vector<CGIInterpreter> t_cgi_conf;

struct CGIInterpreter
{
	static t_cgi_conf ACCEPTED_EXT;
	static const std::string INTERPRETERS_PATH;
	static void load_interpreters();

	CGIInterpreter();
	CGIInterpreter(std::string interpreter, std::vector<std::string> extensions);

	std::string interpreter;
	std::vector<std::string> extensions;

	static std::string getInterpreterForExtension(const std::string& ext);

	void reset();
};

std::ostream& operator<<(std::ostream& os, CGIInterpreter const& cgi);
std::ostream& operator<<(std::ostream& os, const t_cgi_conf& conf);

#endif
