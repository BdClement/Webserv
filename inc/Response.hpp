/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 16:02:43 by clbernar          #+#    #+#             */
/*   Updated: 2024/07/03 14:19:02 by clbernar         ###   ########.fr       */
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

	void				generateResponse(Request & request, ServerConfig & serverBlock, int loc_index);
	void				generateRedirectionResponse(Request & request, Location & locationBlock);
	void				generateStatusLine(Request & request);
	void				generateHeaders(Request & request);
	void				generateErrorBody(Request & request);// Pour gerer les error Page specifiees
	void				findErrorPage(ServerConfig & serverBlock, int error_code, std::string & root);
	void				setContentType(std::string & uri);

	void				clear();

	private:

	std::vector<unsigned char>	m_response;
	mapIntString				m_error_file;
	mapIntString				m_code_meaning;
	unsigned int				m_body_size;
	std::string					m_content_type;
	std::string					m_error_page;

	friend class Handler;
	friend class Request;
};
