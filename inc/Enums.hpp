/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Enums.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irozhkov <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/16 18:24:40 by irozhkov          #+#    #+#             */
/*   Updated: 2025/09/13 14:47:22 by irozhkov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ENUMS_HPP
# define ENUMS_HPP

enum AutoIndexState {
    AINDX_DEF_OFF,
    AINDX_DEF_ON,

    AINDX_SERV_OFF,
    AINDX_SERV_ON,

    AINDX_LOC_OFF,
    AINDX_LOC_ON
};

enum CGIStatus {
	CGI_INIT,
	CGI_RUNNING,
	CGI_WRITING_BODY,
	CGI_READING_OUTPUT,
	CGI_FINISHED,
	CGI_ERROR
};


#endif
