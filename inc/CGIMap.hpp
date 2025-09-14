/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIMap.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/17 12:15:37 by irozhkov          #+#    #+#             */
/*   Updated: 2025/09/14 13:33:59 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIMAP_HPP
# define CGIMAP_HPP

# include <map>
# include <string>
# include <stdexcept>

class CGIMap
{
	private:
		std::map<std::string, std::string> _cgi_map;

	public:
		CGIMap();

		const std::map<std::string, std::string>& getCGIMap() const;
		std::string getInterpreter(const std::string& extension) const;

		bool hasExtension(const std::string& extension) const;
		
		void setInterpreter(const std::string& extension, const std::string& interpreter);
};

#endif
