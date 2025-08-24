/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIEnv.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/17 13:27:31 by irozhkov          #+#    #+#             */
/*   Updated: 2025/08/23 15:25:48 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIENV_HPP
# define CGIENV_HPP

# include <map>
# include <cstring>
# include <cstdlib>
# include <string>

class CGIEnv
{
	private:
		std::map<std::string, std::string>  _cgi_env;
		char** 								_envp;
		size_t 								_size;

		void rebuildCGIEnvp();

	public:
		CGIEnv();
		~CGIEnv();

		std::string getCGIEnvValue(const std::string &key) const;
		const std::map<std::string, std::string>& getCGIEnv() const;

		char** getEnvp() const;

		void setEnvValue(const std::string &key, const std::string &value);

		void clear();
};

#endif
