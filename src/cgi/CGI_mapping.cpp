/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI_mapping.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/10 15:08:30 by irozhkov          #+#    #+#             */
/*   Updated: 2025/08/10 15:31:19 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI_mapping.hpp"

#include <fstream>
#include <iostream>
#include <unistd.h>
#include <limits.h>

CGIMapping::CGIMapping() {}

std::string CGIMapping::getCurrentWorkingDir()
{
	char	buffer[PATH_MAX];

	if (getcwd(buffer, PATH_MAX) == NULL)
	{
		throw std::runtime_error("Cannot get current working directory");
	}

	return (std::string(buffer));
}

std::string CGIMapping::makeAbsolutePath(const std::string& cwd, const std::string& relative_path)
{
    if (relative_path.empty())
	{
		return (cwd);
	}

    if (relative_path[0] == '/')
	{
		return (relative_path);
	}

    if (cwd[cwd.size() - 1] == '/')
	{
		return (cwd + relative_path);
	}
    else
	{
        return (cwd + "/" + relative_path);
	}
}

CGIMapping& CGIMapping::instance() 
{
    static CGIMapping	instance;

    return (instance);
}

void CGIMapping::load(const std::string& filename)
{
    mapping.clear();

    std::ifstream file(filename.c_str());

    if (!file)
	{
		throw std::runtime_error("Cannot open mapping file " + filename);
    }

    std::string cwd = getCurrentWorkingDir();
    if (cwd.empty())
	{
        throw std::runtime_error("Cannot get current working directory");
    }

    std::string line;

    while (std::getline(file, line)) 
	{
        if (line.empty() || line[0] == '#') continue;

        std::size_t pos = line.find('=');

        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        std::string abs_path = makeAbsolutePath(cwd, value);

        mapping[key] = abs_path;
    }
}

std::string CGIMapping::getScriptPath(const std::string& url) const
{
    std::map<std::string, std::string>::const_iterator it = mapping.find(url);

    if (it != mapping.end())
        return it->second;
    else
        return "";
}
