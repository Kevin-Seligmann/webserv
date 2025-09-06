/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIArg.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/06 14:12:34 by irozhkov          #+#    #+#             */
/*   Updated: 2025/09/06 17:18:14 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIARG_HPP
# define CGIARG_HPP

# pragma once
# include <cstdlib>
# include <cstring>
# include <map>

# include "CGIEnv.hpp"
# include "CGIMap.hpp"

class CGIArg
{
	private:
		CGIMap	_map;
		char**	_args;
		size_t	_size;


	public:
		CGIArg(const CGIEnv& env);
		~CGIArg();

		char** getArgs() const; 
};

#endif
