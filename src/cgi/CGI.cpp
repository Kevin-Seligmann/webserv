/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mvisca-g <mvisca-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 15:21:11 by irozhkov          #+#    #+#             */
/*   Updated: 2025/09/22 16:55:56 by mvisca-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

CGI::CGI(const HTTPRequest& req, const VirtualServersManager& server) : _env()
{
	_cgi_status = CGI_INIT;



	_req_pipe[0] = -1;
	_req_pipe[1] = -1;
	_cgi_pipe[0] = -1;
	_cgi_pipe[1] = -1;
	_pid = -1;

}

void CGI::init(const HTTPRequest &req, const VirtualServersManager& server, std::string const & path)
{
	buildEnv(req, server, path);
}

CGI::~CGI()
{
	if (_req_pipe[0] != -1)
	{
		close(_req_pipe[0]);
		_req_pipe[0] = -1;
	}
	if (_req_pipe[1] != -1)
	{
		close(_req_pipe[1]);
		_req_pipe[1] = -1;
	}
	if (_cgi_pipe[0] != -1) 
	{
		close(_cgi_pipe[0]);
		_cgi_pipe[0] = -1;
	}
	if (_cgi_pipe[1] != -1)
	{
		close(_cgi_pipe[1]);
		_cgi_pipe[1] = -1;
	}

	int	status;
	if (_pid > 0) { waitpid(_pid, &status, WNOHANG); }
}

std::string CGI::methodToString(HTTPMethod method) const 
{
	switch (method)
	{
		case GET:
			return ("GET");
		case POST:
			return ("POST");
		case PUT:
			return ("PUT");
		case DELETE:
			return ("DELETE");
		default:
			return ("UNKNOWN");
	}
}

std::map<std::string, std::string> CGI::pathToBlocks(const std::string& path, const std::string &file_path) const
{
	std::map<std::string, std::string> cgi;
	std::vector<std::string> parts;
	std::vector<std::string> exts;

	for (t_cgi_conf::const_iterator it = CGIInterpreter::ACCEPTED_EXT.begin();
		 it != CGIInterpreter::ACCEPTED_EXT.end(); ++it)
	{
		exts.insert(exts.end(), it->extensions.begin(), it->extensions.end());
	}

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

	std::cout << "PATH: " << file_path << std::endl;
	cgi["PATH_INFO"] = "/";

	return (cgi);
}

void CGI::buildEnv(const HTTPRequest& req, const VirtualServersManager& server, std::string const & path)
{
	std::map<std::string, std::string> res = pathToBlocks(req.uri.path, req.get_path());

	_env.setEnvValue("REDIRECT_STATUS", "200");

	// TESTING
	// Construiccion del path de cgi script
	std::string abs_path = path;
	if (abs_path[0] != '/' && abs_path.find("./") == 0)
	{
		char cwd[1024];
		if (getcwd(cwd, sizeof(cwd)))
		{
			abs_path = std::string(cwd) + "/" + abs_path;
		}
	}
	_env.setEnvValue("SCRIPT_FILENAME", path);

	if (!req.body.content.empty())
	{
		std::ostringstream ss;
		ss << req.headers.content_length;
		_env.setEnvValue("CONTENT_LENGTH", ss.str());
		_env.setEnvValue("CONTENT_TYPE", req.headers.content_type.type + '/' + req.headers.content_type.subtype);
		_req_body = req.body.content;
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
//		_env.setEnvValue("SERVER_NAME", server->server_names[0]); // TODO check if you have to use getter
	}

	if (req.headers.port != -1)
	{
		std::ostringstream ss;
		ss << req.headers.port;
		_env.setEnvValue("SERVER_PORT", ss.str());
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

void CGI::runCGI()
{
	_cgi_status = CGI_RUNNING;

	if (pipe(_req_pipe) < 0 || pipe(_cgi_pipe) < 0)
	{
		// TEST 
		perror("pipe failed"); // .
		_cgi_status = CGI_ERROR;
		_cgi_response.buildInternalErrorResponse();
		return ;
	}
	_pid = fork();
	

	if (_pid < 0)
	{
		// TEST 
		perror("pipe failed"); // .
		_cgi_status = CGI_ERROR;
		_cgi_response.buildInternalErrorResponse();
		return ;
	}

	if (_pid == 0)
	{
		// TEST 
		std::cerr << "CHILD: Starting, PID=" << getpid() << std::endl;
		// .

		if (dup2(_req_pipe[0], STDIN_FILENO) == -1 || 
			dup2(_cgi_pipe[1], STDOUT_FILENO) == -1)
		{
			// TEST
			std::cerr << "CHILD: dup2 failed" << std::endl;
			perror("dup2 failed"); // .

			_cgi_status = CGI_ERROR;
			_cgi_response.buildInternalErrorResponse();
			_exit(1);
		}

		close(_req_pipe[1]);
		close(_cgi_pipe[0]);

		CGIArg	arg(_env);
		char** argv = arg.getArgs();

		char** envp = _env.getEnvp();

		// for (int i = 0; i < 100; i ++){
		// 	if (!argv[i])
		// 		break;
		// 	std::cout << argv[i] << std::endl;
		// }

		// for (int i = 0; i < 100; i ++){
		// 	if (!envp[i])
		// 		break;
		// 	std::cout << envp[i] << std::endl;
		// }

		std::string script_path = _env.getCGIEnvValue("SCRIPT_FILENAME");
		// TEST
		std::cerr << "CHILD: Checking script path: [" << script_path << "]" << std::endl;
		std::cerr << "CHILD: Script exists: " << (access(script_path.c_str(), F_OK) == 0 ? "YES" : "NO") << std::endl;
		std::cerr << "CHILD: Script executable: " << (access(script_path.c_str(), X_OK) == 0 ? "YES" : "NO") << std::endl;
		// .

		if (access(script_path.c_str(), X_OK) != 0) {
			// TEST
			std::cerr << "CHILD: Script not executable, exiting" << std::endl;
    		perror("access failed");
			// .
			_exit(1);
		}
		
		// TEST
		std::cerr << "CHILD: About to execve " << argv[0] << std::endl;\
		// .
		
		execve(argv[0], argv, envp);
	}
	else
	{
		close(_req_pipe[0]);
		close(_cgi_pipe[1]);

		if (!_req_body.empty())
		{
			ssize_t written = write(_req_pipe[1], _req_body.c_str(), _req_body.size());
			if (written == -1) 
			{
				_cgi_status = CGI_ERROR;
				_cgi_response.buildInternalErrorResponse();
				return ;
			}
		}
	}
		close(_req_pipe[1]);

		_cgi_status = CGI_WRITING_BODY;

		char buffer[4096];
		ssize_t n;
		std::string cgi_output;
		while ((n = read(_cgi_pipe[0], buffer, sizeof(buffer))) > 0)
		{
			cgi_output.append(buffer, n);
		}

		std::cout << "CGI OUTPUT LENGTH: " << cgi_output.length() << std::endl;
		std::cout << "CGI OUTPUT CONTENT: [" << cgi_output << "]" << std::endl;

		close(_cgi_pipe[0]);

		// TEST
		std::cout << "PARENT: About to waitpid for " << _pid << std::endl;
		int	status_cgi;
		pid_t result = waitpid(_pid, &status_cgi, 0);
		Logger::getInstance() << "\n\n WAITPID RESULT : " << result << "  STATUS : " << status_cgi << std::endl;
		std::cout << "PARENT: waitpid returned " << result << ", errno=" << errno << std::endl;
    	if (result == -1) {
        	perror("waitpid failed");
    	}
		// . mantener int, waitpid

		_cgi_status = CGI_READING_OUTPUT;

		_cgi_response.parseFromCGIOutput(cgi_output);
		_cgi_response.buildResponse();

		_cgi_status = CGI_FINISHED;
}

const CGIResponse& CGI::getCGIResponse() const
{
	return (_cgi_response);
}

CGIStatus CGI::getStatus() const 
{ 
	return (_cgi_status);
}

void CGI::setStatus(CGIStatus s) 
{ 
	_cgi_status = s;
}
