/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/07 16:02:03 by irozhkov          #+#    #+#             */
/*   Updated: 2025/09/27 20:44:08 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
# define CGI_HPP

# pragma once

# include <algorithm>
# include <fcntl.h>
# include <map>
# include <sstream>
# include <string>
# include <strings.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <unistd.h>
# include <vector>
# include <poll.h>
# include <string.h>

# include "CGIArg.hpp"
# include "CGIEnv.hpp"
# include "CGIInterpreter.hpp"
# include "CGIResponse.hpp"
# include "Enums.hpp"
# include "HTTPRequest.hpp"
# include "ServerConfig.hpp"
# include "ActiveFileDescriptor.hpp"
# include "DebugView.hpp"
# include "StreamRequest.hpp"

class VirtualServersManager;

class CGI
{
	private:
		CGIEnv								_env;
		std::string const *					 _req_body; // Make pointer to save memory
		int 								_req_pipe[2];
		int 								_cgi_pipe[2];
		pid_t 								_pid;

		CGIResponse 						_cgi_response;
		CGIStatus 							_cgi_status;

		const char * 						_wr_body;
		size_t								_wr_body_size;
		size_t								_wr_bytes_sent;
	
		bool								_write_finished;
		bool								_read_finished;

		char  								_rd_buffer[4096];
		std::string							_cgi_output;

		StreamRequest & 					_stream_request;

		bool 								_headers_parsed;
		bool 								_header_stream_buffer_sent;
		std::string							_header_stream_buffer;
		std::string							_parsed_header_stream_buffer;

		std::string methodToString(HTTPMethod method) const;
		std::map<std::string, std::string> pathToBlocks(const HTTPRequest& req) const;
		std::string systemPathToCgi(const std::string &system_path);

	public:

		CGI(StreamRequest & stream_request);
		~CGI();

		void buildEnv(const HTTPRequest &req, const VirtualServersManager& server, std::string const & path, ServerConfig * sconf, Location * loc);
		CGIEnv& getEnv();
		void runCGI(int fd);
		void runCGIStreamed(int fd);

		const CGIResponse& getCGIResponse() const;

		CGIStatus getStatus() const;
		void setStatus(CGIStatus s);

		bool done() const {return _cgi_status == CGI_FINISHED ||( _req_pipe[1] == -1 && _cgi_pipe[0] == -1) ;};
		bool finished_reading(){return _read_finished;};
		bool finished_writing(){return _write_finished;};
		bool error() const {return _cgi_status == CGI_ERROR;};

		void init(const HTTPRequest &req, const VirtualServersManager& server, std::string const & path, ServerConfig * sconf, Location * loc);
		void initStreamed(const HTTPRequest &req, const VirtualServersManager& server, std::string const & path, ServerConfig * sconf, Location * loc);

		void reset();
		void setStatus(CGIStatus status, std::string const & txt);

		int write_fd(){return _req_pipe[1];};
		int read_fd(){return _cgi_pipe[0];};

		void sendResponse();
		void parseStreamHeaders();

		std::vector<ActiveFileDescriptor> getActiveFileDescriptors() const;
};


#endif
