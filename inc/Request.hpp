/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/13 12:23:59 by clbernar          #+#    #+#             */
/*   Updated: 2024/05/22 16:00:31 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"
#include "Config.hpp"
#include "Response.hpp"

class Request
{
	public:
	Request();
	Request(Request const& asign);
	~Request();

	Request & operator=(Request const& equal);

	void	parseRequest();
	size_t	parseRequestLine();
	bool	MethodAllowed(std::string const & method) const;
	bool	UriValid(std::string & uri);
	bool	ProtocolValid(std::string const & protocol) const;
	void	parseHeaders(size_t startHeaders);
	bool	HeaderLineValid(std::string &header);
	bool	isKeepAlive();

	void	processRequest(std::vector<Config> & m_config, Response & response);
	void	processGet(Config & config, Response & response);
	bool	checkRessourceAccessibilty(std::string const & ressource);
	// void	processPost(Config & config);
	void	processDelete(Config & config, Response & response);

	void	clear();


	private:
	std::vector<unsigned char>	m_read;
	bool						m_requestIsComplete;
	std::string					m_method;
	std::string					m_uri;
	bool						m_uriIsADirectory;
	std::string					m_query;
	std::string					m_protocol;
	size_t						m_body_pos;// +4  a jouter pour etre au vrai Body
	int							m_error_code;
	int							m_response_code;
	mapString					m_headers;

	friend class Handler;
	friend class Connection;
	friend class Response;
};

