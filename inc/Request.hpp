/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/13 12:23:59 by clbernar          #+#    #+#             */
/*   Updated: 2024/06/28 14:29:26 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"
#include "ServerConfig.hpp"
#include "Response.hpp"

class Request
{
	public:
	Request();
	Request(Request const& asign);
	~Request();

	Request & operator=(Request const& equal);

	// Classe interne pour la gestion des pipes utiles au CGI
	class PipeHandler
	{
		public :
			PipeHandler();
			~PipeHandler();
			PipeHandler & operator=(PipeHandler const& equal);
			void	closeAndReset(int &pipe);
			void	clear();

		private :
			int		pipe_stdin[2];
			int		pipe_stdout[2];
			int		m_bytes_sent;
			pid_t	m_pid;
			std::string	request_method;
			std::string	script_name;
			std::string	path_info;
			std::string	query_string;
			std::string CType;
			std::string	CLength;
			std::string	server;
			std::string	bin;
			std::string	working_dir;

		friend class Handler;
		friend class Connection;
		friend class Request;
	};

	// PARSING REQUEST
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
	void			setConfig(ServerConfig & serverBlock, int loc_index);
	void			checkBody();
	// Chunked
	void			handleChunked();
	int				hexaToInt(std::string const & toConvert);
	void			lastChunk(std::vector<unsigned char>::iterator & bodyStart, int pos, int size);

	// PROCESS REQUEST
	void			processRequest(ServerConfig & m_config, int loc_index, Response & response);
	// Get
	void			processGet(ServerConfig & config, int loc_index, Response & response);
	bool			checkRessourceAccessibilty(std::string const & ressource);
	bool			processGetDirectory(Location & locationBlock, Response & response);
	void			generateAutoIndex(const std::string& directory_path, Response & response);
	bool 			is_directory(const std::string& path);
	// Delete
	void			processDelete(Response & response);
	// Post
	void			processPost(ServerConfig & config, int loc_index, Response & response);
	void			stockData(std::vector<unsigned char> const & body);
	bool			checkFileUpload();
	void			extractFilename(std::string & toExtract);
	void			uploadFile(std::vector<unsigned char> &boundary_body, std::string & uploadLoc);
	// Multipart/form-data
	void			processPostMultipart(std::string boundary, std::vector<unsigned char> & body,ServerConfig & config, int loc_index);
	void			processPostBoundaryBlock(std::vector<unsigned char> &boundary_block, ServerConfig & config, int loc_index);
	std::string		extractBoundary(std::string & contentTypeValue);
	void			updateHeaders(std::string &boundary_headers);
	void			resetMultipart();

	// PROCESS CGI
	void			processCGI(ServerConfig & config, int loc_index);
	void			setEnv(char **env);
	void			setArg(char **arg, std::string & interpreter);
	bool			setPipe();
	void			cgiChildProcess(char **env, char **arg);
	void			cgiParentProcess(pid_t pid);

	// MAINTENANCE
	bool			isKeepAlive();
	void			clear();


	private:
	std::vector<unsigned char>	m_read;
	std::vector<unsigned char>	m_chunked_body;
	bool						m_ready;
	bool						m_chunked;
	bool						m_end_of_chunked;
	std::string					m_method;
	std::string					m_uri;
	bool						m_uriIsADirectory;
	std::string					m_query;
	std::string					m_protocol;
	size_t						m_body_pos;// +4  a jouter pour etre au vrai Body
	int							m_error_code;
	int							m_response_code;
	mapString					m_headers;
	bool						m_cgi;
	std::string					m_filename;
	PipeHandler					m_pipe;
	std::string					m_root;
	bool						m_redirection;

	friend class Handler;
	friend class Connection;
	friend class Response;
};
