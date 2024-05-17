/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 16:02:43 by clbernar          #+#    #+#             */
/*   Updated: 2024/05/17 18:06:21 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"
#include "Config.hpp"

class Request;

class Response
{
	public:
	Response();
	Response(Response const& asign);
	~Response();

	Response & operator=(Response const& equal);

	void	generateResponse(std::vector<Config> & m_config, Request & request);
	void	generateErrorResponse(int error_code) const;
	void	generateStatusLine();

	private:


	// friend class Handler;
	// friend class Connection;
	// friend class Request;
};
