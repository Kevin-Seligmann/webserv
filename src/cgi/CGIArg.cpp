/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIArg.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/06 14:23:15 by irozhkov          #+#    #+#             */
/*   Updated: 2025/09/06 17:17:57 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIArg.hpp"

CGIArg::CGIArg(const CGIEnv& env) : _args(NULL), _size(0)
{
{
    std::string script = env.getCGIValue("SCRIPT_NAME");
    std::string path;

    for (std::map<std::string, std::string>::const_iterator it = _map._cgi_map.begin(); 
		 it != _map._cgi_map.end(); ++it)
	{
		if (script.find(it->first) != std::string::npos)
		{
			path = it->second;
			break;
		}
	}

	if (path.empty()) { path = script; }

	_size = 3;
	_args = new char*[_size];

	_args[0] = strdup(path.c_str());
	_args[1] = strdup(script.c_str());
	_args[2] = NULL;	
}

CGIArg::~CGIArg()
{
	if (_args)
	{
		for (size_t i = 0; i < _size; ++i)
			free(_args[i]);
		delete[] _args;
	}
}

char** CGIArg::getArgs() const
{
	return (_args);
}
