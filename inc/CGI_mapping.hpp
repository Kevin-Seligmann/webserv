/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI_mapping.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/10 15:05:43 by irozhkov          #+#    #+#             */
/*   Updated: 2025/08/10 15:07:51 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_MAPPING_HPP
# define CGI_MAPPING_HPP

#include <string>
#include <map>

class CGIMapping
{
	private:
		std::map<std::string, std::string> mapping;

		CGIMapping();

		std::string getCurrentWorkingDir();
		std::string makeAbsolutePath(const std::string& cwd, const std::string& relative_path);

	public:
		static CGIMapping& instance();

		void load(const std::string& filename);
		std::string getScriptPath(const std::string& url) const;

	private:
		CGIMapping(const CGIMapping&);
		CGIMapping& operator=(const CGIMapping&);
};

#endif
