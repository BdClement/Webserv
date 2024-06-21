/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/08 13:08:32 by bmirlico          #+#    #+#             */
/*   Updated: 2024/06/21 14:12:44 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "ServerConfig.hpp"

class ServerConfig;

class ConfigParser
{
	private:
		int	_nbServers;
		std::vector<ServerConfig> _servers;
		std::vector<std::string>	_serverConfigs;
	public:
		ConfigParser(void);
		~ConfigParser(void);
		ConfigParser(const ConfigParser &cpy);
		ConfigParser& operator=(const ConfigParser &rhs);
		bool parse(const std::string &configFile);
		void createServers(const std::string &configFile);
		void removeComments(std::string &content);
		void removeSpaces(std::string &content);
		void createServerBlocks(std::string &content);
		size_t findStartServerBlock(size_t start, std::string &content);
		size_t findEndServerBlock(size_t start, std::string &content);
		std::vector<std::string> splitServerBlock(std::string line, std::string sep);
		void createServer(std::string &config, ServerConfig &server);
		void checkDupServers(void);
		std::vector<ServerConfig> getServers() const;
		void printServers(void);
		public:
		class ErrorException: public std::exception
		{
			private:
				std::string _message;
			public:
				ErrorException(std::string message) throw()
				{
					_message = "CONFIG PARSER ERROR: " + message;
				}
				virtual const char* what() const throw()
				{
					return (_message.c_str());
				}
				virtual ~ErrorException() throw () {}
		};
	friend class Handler;
};

#endif
