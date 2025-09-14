/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigInheritance.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/16 16:42:00 by irozhkov          #+#    #+#             */
/*   Updated: 2025/08/16 19:59:02 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGINHERITANCE_HPP
# define CONFIGINHERITANCE_HPP

# include "Parsed.hpp"

void	applyAutoindex(ParsedServer& server);
void	applyAllowMethods(ParsedServer& server);
void	applyIndexFiles(ParsedServer& server);

#endif
