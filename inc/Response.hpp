/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 16:02:43 by clbernar          #+#    #+#             */
/*   Updated: 2024/06/24 17:55:56 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"
// #include "Config.hpp"
#include "ServerConfig.hpp"// ???

class Request;

class Response
{
	public:
	Response();
	Response(Response const& asign);
	~Response();

	Response & operator=(Response const& equal);

	void				set_error_file();
	void				set_code_meaning();

	void				generateResponse(Request & request, ServerConfig & serverBlock);
	void				generateStatusLine(Request & request);
	void				generateHeaders(Request & request);
	void				generateErrorBody(Request & request, ServerConfig & serverBlock);// Pour gerer les error Page specifiees
	void				setContentType(std::string & uri);

	void				clear();

	private:

	std::vector<unsigned char>	m_response;
	mapIntString				m_error_file;
	mapIntString				m_code_meaning;
	unsigned int				m_body_size;
	std::string					m_content_type;

	friend class Handler;
	// friend class Connection;
	friend class Request;
};
