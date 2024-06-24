/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/13 12:23:59 by clbernar          #+#    #+#             */
/*   Updated: 2024/06/24 18:50:33 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"
// #include "Config.hpp"
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
			void	clear(); // Pour passer a une autre requete (notamment les bool)

		private :
			int		pipe_stdin[2];
			int		pipe_stdout[2];
			int		m_bytes_sent;
			pid_t	m_pid;
			std::string	request_method;
			std::string	script_name;
			std::string	query_string;
			std::string CType;
			std::string	CLength;
			std::string	server;
			std::string	ressource;
			std::string	bin;

		friend class Handler;
		friend class Connection;
		friend class Request;
		// friend class Response;
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
	// void			processRequest(std::vector<Config> & m_config, Response & response);
	void			processRequest(std::vector<ServerConfig> & m_config, Response & response);
	// Get
	// void			processGet(Config & config, Response & response);
	void			processGet(ServerConfig & config, Response & response);
	bool			checkRessourceAccessibilty(std::string const & ressource);
	// Delete
	// void			processDelete(Config & config, Response & response);
	void			processDelete(ServerConfig & config, Response & response);
	// Post
	// void			processPost(Config & config, Response & response);
	void			processPost(ServerConfig & config, Response & response);
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

	// PROCESS CGI
	void			processCGI(ServerConfig & config);
	void			setEnv(char **env);
	void			setArg(char **arg);
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

	friend class Handler;
	friend class Connection;
	friend class Response;
};
