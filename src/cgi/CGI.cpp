/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 15:21:11 by irozhkov          #+#    #+#             */
/*   Updated: 2025/08/30 16:50:52 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

CGI::CGI(const HTTPRequest& req, const ParsedServer& server)
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

std::map CGI::pathToBlocks(const std::string& path) const
{
	std::map<std::string, std::string>	cgi;
	size_t	start;
	size_t	last_slash;
	size_t	temp_slash;

	last_slash = path.find_last_of('/');

	if (last_slash == std::string::npos) { start = 0; }
	else { start = last_slash + 1; }

	temp_slash = path.find('/', start);

	if (temp_slash == std::string::npos)
	{
		cgi["SCRIPT_NAME"] = path.substr(start);
		cgi["PATH_INFO"] = "";
	}
	else
	{
		cgi["SCRIPT_NAME"] = path.substr(start, temp_slash - start);
		cgi["PATH_INFO"] = path.substr(temp_slash);
	}

	return (cgi);
}

void CGI::buildEnv(const HTTPRequest& req, const ParsedServer& server)
{
	std::map<std::string, std::string> res = pathToBlocks(req.uri.path);

	if (!req.body.empty())
	{
		_env.setEnvValue("CONTENT_LENGTH", std::to_string(req.body.size()));
//		auto it = req.headers.find("Content-Type");
//		if (it != req.headers.end())
//			_env["CONTENT_TYPE"] = it->second;
	}
	
	_env.setEnvValue("GATEWAY_INTERFACE", "CGI/1.1"); // default

	if (req.uri.query.empty()) { _env.setEnvValue("QUERY_STRING", ""); }
	else { _env.setEnvValue("QUERY_STRING", req.uri.query); }

	_env.setEnvValue("PATH_INFO", res["PATH_INFO"]);
	_env.setEnvValue("REQUEST_METHOD", methodToString(req.method));
	_env.setEnvValue("SCRIPT_NAME", res["SCRIPT_NAME"]);
	_env.setEnvValue("SERVER_NAME", "server"); // TODO from soket
	_env.setEnvValue("SERVER_PORT", "8080"); // TODO CGIUtils::socketToPort(con.getSocketFd());
	_env.setEnvValue("SERVER_PROTOCOL", req.protocol);
	_env.setEnvValue("SERVER_SOFTWARE", "webserver"); // default
 
}

CGIEnv& CGI::getEnv()
{
    return (_env);
}
