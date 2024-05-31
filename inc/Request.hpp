/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/13 12:23:59 by clbernar          #+#    #+#             */
/*   Updated: 2024/05/31 20:03:05 by clbernar         ###   ########.fr       */
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

	// PARSING
	void			parseRequest();
	//	RequestLine
	size_t			parseRequestLine();
	bool			MethodAllowed(std::string const & method) const;
	bool			UriValid(std::string & uri);
	bool			ProtocolValid(std::string const & protocol) const;
	// Headers
	void			parseHeaders(size_t startHeaders);
	bool			HeaderLineValid(std::string &header);
	bool			checkContentLengthValue(std::string & value, long long &content_length);
	// Body
	void			checkBody();

	// PROCESS REQUEST
	void			processRequest(std::vector<Config> & m_config, Response & response);
	// Get
	void			processGet(Config & config, Response & response);
	bool			checkRessourceAccessibilty(std::string const & ressource);
	// Delete
	void			processDelete(Config & config, Response & response);
	// Post
	void			processPost(Config & config, Response & response);
	void			stockData(std::vector<unsigned char> const & body);
	bool			checkFileUpload();// a voir si je ne retourne pas une string pour ne pas srucharger ma request
	void			extractFilename(std::string & toExtract);
	void			uploadFile(std::vector<unsigned char> &boundary_body);
	// Multipart/form-data
	void			processPostMultipart(std::string boundary, std::vector<unsigned char> & body);
	void			processPostBoundaryBlock(std::vector<unsigned char> &boundary_block);
	std::string		extractBoundary(std::string & contentTypeValue);
	void			updateHeaders(std::string &boundary_headers);
	void			resetMultipart();

	// MAINTENANCE
	bool			isKeepAlive();
	void			clear();


	private:
	std::vector<unsigned char>	m_read;
	bool						m_parsed;
	std::string					m_method;
	std::string					m_uri;
	bool						m_uriIsADirectory;
	std::string					m_query;
	std::string					m_protocol;
	size_t						m_body_pos;// +4  a jouter pour etre au vrai Body
	int							m_error_code;
	int							m_response_code;
	mapString					m_headers;
	std::string					m_filename;

	friend class Handler;
	friend class Connection;
	friend class Response;
};

