/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/07 16:02:03 by irozhkov          #+#    #+#             */
/*   Updated: 2025/09/20 16:10:44 by irozhkov         ###   ########.fr       */
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

# include "CGIArg.hpp"
# include "CGIEnv.hpp"
# include "CGIInterpreter.hpp"
# include "CGIResponse.hpp"
# include "Enums.hpp"
# include "HTTPRequest.hpp"
# include "ServerConfig.hpp"

class VirtualServersManager;

class CGI
{
	private:
		CGIEnv								_env;
		std::string							_req_body;
		int 								_req_pipe[2];
		int 								_cgi_pipe[2];
		pid_t 								_pid;

		CGIResponse 						_cgi_response;
		CGIStatus 							_cgi_status;

		std::string methodToString(HTTPMethod method) const;
		std::map<std::string, std::string> pathToBlocks(const std::string& path) const;

	public:

		CGI(const HTTPRequest& req, const VirtualServersManager& server);
		~CGI();

		void buildEnv(const HTTPRequest &req, const VirtualServersManager& server);
		CGIEnv& getEnv();
		void runCGI();

		const CGIResponse& getCGIResponse() const;

		CGIStatus getStatus() const;
		void setStatus(CGIStatus s);

		void init(const HTTPRequest &req, const VirtualServersManager& server);
};


#endif
