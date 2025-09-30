/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIResponse.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mvisca-g <mvisca-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/24 11:39:00 by irozhkov          #+#    #+#             */
/*   Updated: 2025/09/29 22:41:42 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIRESPONSE_HPP
# define CGIRESPONSE_HPP

# include <cstddef> 
# include <fcntl.h>
# include <iostream>
# include <map>
# include <sstream>
# include <string>
# include <unistd.h>   // write, close

#include "DebugView.hpp"

class CGIResponse
{
	private:
		std::string							_contentType;
		std::string							_status;
		std::string							_location;
		std::map<std::string, std::string>	_cgiResponseHeaders;
		std::ostringstream					_bodyStream;

	   	std::string 						_responseBuffer;
		size_t 								_sentBytes;

	public:

		std::string getContentType() const;
		std::string getStatus() const;
		std::string getLocation() const;
		std::string getCGIResponseHeader(const std::string& key) const;
		const std::map<std::string, std::string>& getCGIResponseHeaders() const;
		const std::string& getResponseBuffer() const;

		void parseFromCGIOutput(const std::string& cgiOutput);
		void buildResponse();
		void buildInternalErrorResponse();
		void buildNotFoundErrorResponse();
		void buildForbiddenErrorResponse();


		void setContentType(const std::string& type);
		void setStatus(const std::string& stat);
		void setLocation(const std::string& loc);
		void setHeader(const std::string& header, const std::string& value);
		void reset();

};


#endif
