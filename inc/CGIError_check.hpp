/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIError_check.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/02 12:49:55 by irozhkov          #+#    #+#             */
/*   Updated: 2025/10/02 12:53:23 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIERROR_CHECK_HPP
# define CGIERROR_CHECK_HPP

# pragma once
# include <string>
# include <sstream>
# include <cctype>
# include <cstddef>

bool hasContentType(const std::string& headers);
int cgiErrorCheck(const std::string& out);

#endif
