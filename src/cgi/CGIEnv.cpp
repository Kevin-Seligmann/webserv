/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIEnv.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 15:21:11 by irozhkov          #+#    #+#             */
/*   Updated: 2025/09/14 14:52:46 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIEnv.hpp"

CGIEnv::CGIEnv() : _envp(NULL), _size(0) {}

CGIEnv::~CGIEnv()
{
    if (_envp)
	{
		for (size_t i = 0; i < _size; ++i)
			free(_envp[i]);
		delete[] _envp;
	}
}

void CGIEnv::rebuildCGIEnvp()
{
	if (_envp)
	{
		for (size_t i = 0; i < _size; ++i)
			free(_envp[i]);
		delete[] _envp;
	}

	_size = _cgi_env.size();
	_envp = new char*[_size + 1];

	size_t i = 0;
	for (std::map<std::string, std::string>::const_iterator it = _cgi_env.begin();
		 it != _cgi_env.end(); ++it, ++i)
	{
		std::string pair = it->first + "=" + it->second;
		_envp[i] = strdup(pair.c_str());
	}
	_envp[_size] = NULL;
}

std::string CGIEnv::getCGIEnvValue(const std::string &key) const
{
	std::map<std::string, std::string>::const_iterator it = _cgi_env.find(key);
	if (it != _cgi_env.end())
		return (it->second);
	return ("");
}

const std::map<std::string, std::string>& CGIEnv::getCGIEnv() const
{
	return (_cgi_env);
}

char** CGIEnv::getEnvp() const
{
	return (_envp);
}

void CGIEnv::setEnvValue(const std::string &key, const std::string &value) {
	_cgi_env[key] = value;
	rebuildCGIEnvp();
}

void CGIEnv::clear()
{
	_cgi_env.clear();
	rebuildCGIEnvp();
}
