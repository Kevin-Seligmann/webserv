/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mvisca-g <mvisca-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/23 15:21:11 by irozhkov          #+#    #+#             */
/*   Updated: 2025/10/04 14:30:51 by mvisca-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

CGI::CGI() : _env()
{
	_cgi_status = CGI_INIT;

	_req_pipe[0] = -1;
	_req_pipe[1] = -1;
	_cgi_pipe[0] = -1;
	_cgi_pipe[1] = -1;
	_pid = -1;
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



std::map<std::string, std::string> CGI::pathToBlocks(const HTTPRequest& req) const
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

	std::stringstream ss(req.get_path());
	std::string item;

	while (std::getline(ss, item, '/'))
	{
		if (!item.empty())
		{
			parts.push_back(item);
		}
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
		if (indx != std::string::npos)
		{
			break;
		}
	}

	if (indx != std::string::npos)
	{
		std::string scriptName = "";
		for (size_t i = 0; i <= indx; ++i)
		{
			scriptName += "/" + parts[i];
		}

		if (scriptName.compare(0, 9, "/cgi-bin/") != 0)
			scriptName = "/cgi-bin" + scriptName;

		cgi["SCRIPT_NAME"] = scriptName;
	}
	else
	{
		std::string path = req.get_path();
		if (path.compare(0, 9, "/cgi-bin/") != 0)
			path = "/cgi-bin" + path;
		cgi["SCRIPT_NAME"] = path;
	}

	std::string pathInfo;
	if (indx != std::string::npos && indx + 1 < parts.size())
	{
		for (size_t k = indx + 1; k < parts.size(); ++k)
			pathInfo += "/" + parts[k];
	}

	bool isTestScript = req.get_path().find(".bla") != std::string::npos;
	// TODO: Confirmar
	if (isTestScript)
	{
    	cgi["PATH_INFO"] = req.get_path();
	}
	else
	{
		cgi["PATH_INFO"] = pathInfo;
	}

	return (cgi);
}

/*
	EJEMPLO SOLICITUD CGI - /directory/youpi.php/path. Local: /etc/var/www/YoupiBanane/Youpi.php 

	PHP_SELF: /directory/youpi.php - Autogenerado
	GATEWAY_INTERFACE: CGI/1.1 - Estático 

	# Meta
	REQUEST_METHOD: GET
	SERVER_PROTOCOL: HTTP/1.1 - Estático
	REQUEST_TIME: timestamp - Generado al enviar la petición CGI

	# Server
	SERVER_ADDR: 127.0.0.1 - Dirección ip del servidor
	SERVER_PORT: 8080 - Puerto del servidor
	SERVER_NAME: localhost/servername - Nombre del servidor (Server name o dominio)
	SERVER_SOFTWARE: webserv - Estático

	# Client
	REMOTE_ADDR: 127.0.0.1 - Dirección IP del cliente
	REMOTE_HOST: localhost - Nombre del cliente
	REMOTE_PORT: 60000 - Puerto de la solicitud

	# Paths
	SCRIPT_FILENAME: /etc/var/www/YoupiBanane/Youpi.php  - Ruta física hacia el documento
	PATH_TRANSLATED: /etc/var/www/YoupiBanane/Youpi.php  - Ruta física del documento
	SCRIPT_NAME: /directory/youpi.php - Nombre lógico del script
	PATH_INFO: /path - Luego del script 
	DOCUMENT_ROOT: /etc/var/www - Raiz física del servidor
	REQUEST_URI: /directory/youpi.php/path - URI Completo
	PATH_INFO_CUSTOM: /directory/youpi.php/path - CUSTOM: URI COMPLETA
*/


std::string CGI::systemPathToCgi(const std::string &system_path)
{
	if (system_path.empty())
		return system_path;

	std::vector<std::string> parts;
	std::string current;
	for (size_t i = 0; i < system_path.size(); ++i) 
	{
		if (system_path[i] == '/')
		{
			if (!current.empty())
			{
				parts.push_back(current);
				current.clear();
			}
		}
		else
		{
			current += system_path[i];
		}
	}

	if (!current.empty())
		parts.push_back(current);

	for (size_t i = 0; i < parts.size(); ++i)
	{
		for (t_cgi_conf::const_iterator it = CGIInterpreter::ACCEPTED_EXT.begin();
			 it != CGIInterpreter::ACCEPTED_EXT.end(); ++it)
		{
			for (std::vector<std::string>::const_iterator ext = it->extensions.begin();
				 ext != it->extensions.end(); ++ext)
			{
				const std::string &extension = *ext;
				if (parts[i].size() >= extension.size() &&
					parts[i].compare(parts[i].size() - extension.size(), extension.size(), extension) == 0)
				{
					parts[i] = "cgi-bin/" + parts[i];

                
					std::string result;
					for (size_t j = 0; j < parts.size(); ++j) 
					{
						result += "/" + parts[j];
					}
					return result;
				}
			}
		}
	}

	return system_path;
}

void CGI::buildEnv(const HTTPRequest& req, const VirtualServersManager& server, std::string const & system_path, ServerConfig * sconf, Location * loc)
{
	for (std::map<std::string, std::string >::const_iterator it = req.headers.fields.begin(); it != req.headers.fields.end(); it ++)
	{
		std::string name = it->first;
		for (std::string::iterator s = name.begin(); s != name.end(); s ++)
		{
			*s = toupper(*s);
			if (*s == '-')
				*s = '_';
		}
		_env.setEnvValue("HTTP_" + name, it->second);
	}

	// META/REQUEST
	_env.setEnvValue("GATEWAY_INTERFACE", "CGI/1.1"); // default
	_env.setEnvValue("REDIRECT_STATUS", "200");
	
	_env.setEnvValue("REQUEST_METHOD", methodToString(req.method));
 	_env.setEnvValue("QUERY_STRING", req.uri.query);
		
	_req_body = & req.body.content;
	if (!req.body.content.empty())
	{
		std::ostringstream ss;
		ss << req.headers.content_length;
		_env.setEnvValue("CONTENT_LENGTH", ss.str());
		_env.setEnvValue("CONTENT_TYPE", req.headers.content_type.type + '/' + req.headers.content_type.subtype);
	}

	if (!req.headers.host.empty())
	{
		_env.setEnvValue("SERVER_NAME", req.headers.host);
	}
	else
	{
		if (!sconf->server_names.empty())
			_env.setEnvValue("SERVER_NAME", sconf->server_names[0]);
		else
			_env.setEnvValue("SERVER_NAME", "_");;

	}

	if (req.body.content.size() > 0)
	{
		_env.setEnvValue("CONTENT_LENGTH", wss::i_to_dec(req.body.content.size() ));
//		_env.setEnvValue("CONTENT_TYPE", req.headers.getContentType());
		_env.setEnvValue("CONTENT_TYPE", req.headers.content_type.getString());
	}

	// CLIENT

	// SERVER
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


	// # Paths
	// SCRIPT_FILENAME: /etc/var/www/YoupiBanane/Youpi.php  - Ruta física hacia el documento
	// PATH_TRANSLATED: /etc/var/www/YoupiBanane/Youpi.php  - Ruta física del documento
	// SCRIPT_NAME: /directory/youpi.php - Nombre lógico del script
	// PATH_INFO: /path - Luego del script 
	// DOCUMENT_ROOT: /etc/var/www - Raiz física del servidor
	// REQUEST_URI: /directory/youpi.php/path - URI Completo
	// PATH_INFO_CUSTOM: /directory/youpi.php/path - CUSTOM: URI COMPLETA
	std::map<std::string, std::string> res = pathToBlocks(req);
	// PATHS

//	std::string cgi_path = systemPathToCgi(system_path);
	_env.setEnvValue("SCRIPT_FILENAME", system_path); 
	_env.setEnvValue("PATH_TRANSLATED", system_path); // realpath()
	_env.setEnvValue("REQUEST_URI", req.get_path());
	_env.setEnvValue("DOCUMENT_ROOT", sconf->getRoot());
	_env.setEnvValue("SCRIPT_NAME", res["SCRIPT_NAME"]);

	// TODO: Mirar extensión
	_env.setEnvValue("PATH_INFO", res["PATH_INFO"]);
	
	(void) server;
	(void) loc;
}

CGIEnv& CGI::getEnv()
{
    return (_env);
}

void CGI::init(const HTTPRequest &req, const VirtualServersManager& server, std::string const & system_path, ServerConfig * sconf, Location * loc)
{
	buildEnv(req, server, system_path, sconf, loc);

	if (pipe(_req_pipe) < 0 || pipe(_cgi_pipe) < 0)
	{
		close(_req_pipe[0]); close(_req_pipe[1]);
		close(_cgi_pipe[0]); close(_cgi_pipe[1]);
		setStatus(CGI_ERROR, "CGI ERROR PIPE: " + std::string(strerror(errno)));
		_cgi_response.buildInternalErrorResponse();
		return ;
	}

	_pid = fork();
	
	if (_pid < 0)
	{
		setStatus(CGI_ERROR, "CGI ERROR FORK: " + std::string(strerror(errno)));
		_cgi_response.buildInternalErrorResponse();
		return ;
	}
	if (_pid == 0)
	{
		close(_req_pipe[1]);
		close(_cgi_pipe[0]);

		if (dup2(_req_pipe[0], STDIN_FILENO) == -1 || 
			dup2(_cgi_pipe[1], STDOUT_FILENO) == -1)
		{
			write(STDOUT_FILENO, "__CGI_ERROR_500__", 17);
			_exit(1);
		}


		close(_req_pipe[0]);
		close(_cgi_pipe[1]);

		CGIArg	arg(_env);

		char** argv = arg.getArgs();
		char** envp = _env.getEnvp();

		if (access(argv[1], F_OK) != 0)
		{
			write(STDOUT_FILENO, "__CGI_ERROR_404__", 17);
			_exit(1);
		}
		
		if (access(argv[1], X_OK) != 0) 
		{
			write(STDOUT_FILENO, "__CGI_ERROR_403__", 17);
            _exit(1);
		}

		execve(argv[0], argv, envp);

		write(STDOUT_FILENO, "__CGI_ERROR_500__", 17);
		_exit(1);
	}
	else 
	{
		close(_req_pipe[0]);
		close(_cgi_pipe[1]);

		// Make pipes not blocking
		int flags = fcntl(_req_pipe[1], F_GETFL, 0);
		fcntl(_req_pipe[1], F_SETFL, flags | O_NONBLOCK);

		flags = fcntl(_cgi_pipe[0], F_GETFL, 0);
		fcntl(_cgi_pipe[0], F_SETFL, flags | O_NONBLOCK);


		// Initialize reading and writing data
		// Write setup
		_wr_body = _req_body->c_str();
		_wr_body_size = _req_body->size();
		_wr_bytes_sent = 0;
		_write_finished = false;

		// Read setup
		_read_finished = false;

		if (_wr_body_size == 0)
		{
			_write_finished = true;
			close(_req_pipe[1]);
		}
		setStatus(CGI_RUNNING, "RUNNING CGI");
	}
}

void CGI::runCGI(int fd)
{
	if (fd == _req_pipe[1])
	{
		ssize_t written = write(_req_pipe[1], _wr_body + _wr_bytes_sent, std::min(_wr_body_size - _wr_bytes_sent, 8000UL));
		if (written >= 0)
		{
			_wr_bytes_sent += written;
			if (_wr_bytes_sent >= _wr_body_size)
			{
				_write_finished = true;
				close(_req_pipe[1]);
			}
		}
	}
	else if (fd == _cgi_pipe[0])
	{
		ssize_t readed = read(_cgi_pipe[0], _rd_buffer, sizeof(_rd_buffer));
		if (readed > 0)
		{
			_cgi_output.append(_rd_buffer, readed);
		}
		else if (readed == 0)
		{
			_read_finished = true;
		}
	}

	if (_read_finished && _write_finished)
	{
		close(_req_pipe[1]);
		close(_cgi_pipe[0]);

		if (_cgi_output.find("__CGI_ERROR_404__") != std::string::npos) {
			_cgi_response.buildNotFoundErrorResponse();
			setStatus(CGI_ERROR, "CGI Script not found");
		}
		else if (_cgi_output.find("__CGI_ERROR_403__") != std::string::npos) {
			_cgi_response.buildForbiddenErrorResponse();
			setStatus(CGI_ERROR, "CGI Script permission denied");
		}
		else if (_cgi_output.find("__CGI_ERROR_500__") != std::string::npos) {
			_cgi_response.buildInternalErrorResponse();
			setStatus(CGI_ERROR, "CGI Execve failed");
		}
		else {
			_cgi_response.parseFromCGIOutput(_cgi_output);
			_cgi_response.buildResponse();
			setStatus(CGI_FINISHED, "CGI FINISHED");
		}
}	
/*	else 
	{
		CODE_ERR("Imposible CGI status");
	}

	if (_read_finished && _write_finished)
	{
		close(_req_pipe[1]);
		close(_cgi_pipe[0]);

		// int status_check;
		// pid_t check_result = waitpid(_pid, &status_check, WNOHANG);
		// status_check = status_check >> 8;

		// if (check_result > 0 && status_check > 0)
		// {
		// 	_cgi_response.buildInternalErrorResponse();
		// 	setStatus(CGI_ERROR, "CGI Error: " + wss::i_to_dec(status_check));
		// }
		// else 
		// {
			_cgi_response.parseFromCGIOutput(_cgi_output);
			_cgi_response.buildResponse();
			setStatus(CGI_FINISHED, "CGI FINISHED");
		// }
	}*/
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

std::vector<ActiveFileDescriptor> CGI::getActiveFileDescriptors() const
{
	std::vector<ActiveFileDescriptor> fds;

	if (_cgi_status == CGI_RUNNING)
	{
		if (!_read_finished)
			fds.push_back(ActiveFileDescriptor(_cgi_pipe[0], POLLIN));
		if (!_write_finished)
			fds.push_back(ActiveFileDescriptor(_req_pipe[1], POLLOUT));
	}
	else
		CODE_ERR("Trying to get active FDs from an invalid status");
	return fds;
}

void CGI::reset()
{
    if (_pid > 0) {
        int status;
        waitpid(_pid, &status, 0);
    }

	setStatus(CGI_INIT, "CGI IDLE");
	_req_pipe[0] = -1;
	_req_pipe[1] = -1;
	_cgi_pipe[0] = -1;
	_cgi_pipe[1] = -1;
	_pid = -1;
	std::string().swap(_cgi_output);
	_cgi_response.reset();
}


void CGI::setStatus(CGIStatus status, std::string const & txt)
{
	Logger::getInstance() << "CGI set status " << (int) status << " " << txt << std::endl;
	_cgi_status = status;
}
