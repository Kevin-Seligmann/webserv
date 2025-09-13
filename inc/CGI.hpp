/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/07 16:02:03 by irozhkov          #+#    #+#             */
/*   Updated: 2025/09/13 13:44:41 by irozhkov         ###   ########.fr       */
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
# include "CGIResponse.hpp"
# include "Enums.hpp"
# include "HTTPRequest.hpp"

class CGI
{
	private:
		CGIEnv								_env;
		std::string							_req_body;
		int 								_req_pipe[2];
		int 								_cgi_pipe[2];
		pid_t 								_pid;

		CGIStatus 							_cgi_status;

		std::string methodToString(HTTPMethod method) const;
		std::map<std::string, std::string> pathToBlocks(const std::string& path) const;

	public:

		CGI(int client_fd, const HTTPRequest& req, const ServerConfig* server);
		~CGI();

		void buildEnv(const HTTPRequest &req, const ServerConfig* server);
		CGIEnv& getEnv();
		void runCGI();

		CGIStatus getStatus() const;
		void setStatus(CGIStatus s);
};


#endif
