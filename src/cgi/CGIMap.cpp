/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIMap.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 15:06:24 by irozhkov          #+#    #+#             */
/*   Updated: 2025/08/23 15:20:12 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIMap.hpp"

CGIMap::CGIMap()
{
	_cgi_map[".py"]  = "/usr/bin/python3";
	_cgi_map[".php"] = "/usr/bin/php-cgi";
}

const std::map<std::string, std::string>& CGIMap::getCGIMap() const
{
	return (_cgi_map);
}

std::string CGIMap::getInterpreter(const std::string& extension) const
{
	std::map<std::string, std::string>::const_iterator it = _cgi_map.find(extension);
	if (it != _cgi_map.end())
		return (it->second);
	throw std::runtime_error("No CGI handler for extension: " + extension);
}

bool CGIMap::hasExtension(const std::string& extension) const
{
	std::map<std::string, std::string>::const_iterator it = _cgi_map.find(extension);
	if (it != _cgi_map.end())
		return (true);
	throw std::runtime_error("Extension is not in server map: " + extension);
}

void	CGIMap::setInterpreter(const std::string& extension, const std::string& interpreter)
{
	_cgi_map[extension] = interpreter;
}
