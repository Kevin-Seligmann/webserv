/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 15:21:11 by irozhkov          #+#    #+#             */
/*   Updated: 2025/08/31 19:55:26 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

CGI::CGI(int cliend_fd, const HTTPRequest& req, const ServerConfig* server)
{
	buildEnv(req, server);
//	pid = -1;

}

std::string CGI::methodToString(HTTPMethod method) const 
{
	switch (method)
	{
		case HTTPMethod::GET:
			return ("GET");
		case HTTPMethod::POST:
			return ("POST");
		case HTTPMethod::PUT:
			return ("PUT");
		case HTTPMethod::DELETE:
			return ("DELETE");
		default:
			return ("UNKNOWN");
	}
}

std::map<std::string, std::string> CGI::pathToBlocks(const std::string& path) const
{
	std::map<std::string, std::string> cgi;
	std::vector<std::string> parts;
	std::vector<std::string> exts;

	exts.push_back(".cgi");
	exts.push_back(".js");
	exts.push_back(".php");
	exts.push_back(".py");

	size_t indx = std::string::npos;

	std::stringstream ss(path);
	std::string item;
	while (std::getline(ss, item, '/'))
	{
		if (!item.empty()) { parts.push_back(item); }
	}

	for (size_t i = 0; i < parts.size(); ++i)
	{
		for (size_t j = 0; j < exts.size(); ++j)
		{
			size_t hasScript = parts[i].find(exts[j]);
			if (hasScript != std::string::npos)
			{
				indx = i;
				break;
			}
		}

		if (indx != std::string::npos) { break; }
	}

	if (indx != std::string::npos) {cgi["SCRIPT_NAME"] = parts[indx]; }
	else { cgi["SCRIPT_NAME"] = ""; }

	std::string pathInfo;
	if (indx != std::string::npos && indx + 1 < parts.size())
	{
		for (size_t k = indx + 1; k < parts.size(); ++k)
			pathInfo += "/" + parts[k];
	}
	cgi["PATH_INFO"] = pathInfo;

	return (cgi);
}

void CGI::buildEnv(const HTTPRequest& req, const ServerConfig* server)
{
	std::map<std::string, std::string> res = pathToBlocks(req.uri.path);

	if (!req.body.empty())
	{
		_env.setEnvValue("CONTENT_LENGTH", std::to_string(req.headers.content_length));
		_env.setEnvValue("CONTENT_TYPE", req.headers.content_type.type + '/' + req.headers.content_type.subtype);
	}
	
	_env.setEnvValue("GATEWAY_INTERFACE", "CGI/1.1"); // default

	if (req.uri.query.empty()) { _env.setEnvValue("QUERY_STRING", ""); }
	else { _env.setEnvValue("QUERY_STRING", req.uri.query); }

	_env.setEnvValue("PATH_INFO", res["PATH_INFO"]);
	_env.setEnvValue("REQUEST_METHOD", methodToString(req.method));
	_env.setEnvValue("SCRIPT_NAME", res["SCRIPT_NAME"]);

	if (!req.headers.host.empty())
	{
		_env.setEnvValue("SERVER_NAME", req.headers.host);
	}
	else
	{
		_env.setEnvValue("SERVER_NAME", server.server_names[0]); // TODO check if you have to use getter
	}

	if (req.headers.port != -1)
	{
		_env.setEnvValue("SERVER_PORT", req.headers.port);
	}
	else
	{
		_env.setEnvValue("SERVER_PORT", "8080"); // TODO check how to get port from location OR server config
	}

	_env.setEnvValue("SERVER_PROTOCOL", req.protocol);
	_env.setEnvValue("SERVER_SOFTWARE", "webserver"); // default
 
}

CGIEnv& CGI::getEnv()
{
    return (_env);
}
