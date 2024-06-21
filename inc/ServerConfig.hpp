/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/19 19:12:53 by bmirlico          #+#    #+#             */
/*   Updated: 2024/06/21 13:21:39 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "Location.hpp"

class Location;

class ServerConfig
{
	private:
		uint16_t _port;
		std::string _host;
		std::string _serverName;
		std::string _root;
		unsigned long _clientMaxBodySize;
		std::string _index;
		bool _autoIndex;
		std::map<int, std::string> _errorPages;
		std::vector<Location> _locations;
	public:
		ServerConfig(void);
		ServerConfig(const ServerConfig &cpy);
		ServerConfig& operator=(const ServerConfig &rhs);
		~ServerConfig(void);

		void initErrorPages(void);

		void setHost(std::string host);
		void setPort(std::string input);
		void setServerName(std::string serverName);
		void setRoot(std::string root);
		void setClientMaxBodySize(std::string input);
		void setErrorPages(std::vector<std::string> &errorPages);
		std::string statusCodeString(int statusCode);
		void setIndex(std::string index);
		void setAutoindex(std::string autoindex);
		void setLocation(std::string nameLocation, std::vector<std::string> parametr);

		bool isInteger(std::string& s);
		bool isValidHost(std::string host);
		bool isValidErrorPages(void);
		//int isValidLocation(Location &location) const; fonction supprimée car Clément vérifie toutes les permissions dans sa partie

		const std::string &getServerName();
		const uint16_t &getPort();
		const std::string &getHost();
		const size_t &getClientMaxBodySize();
		const std::vector<Location> &getLocations();
		const std::string &getRoot();
		const std::map<int, std::string> &getErrorPages();
		const std::string &getIndex();
		const bool &getAutoindex();
		//const std::string &getPathErrorPage(int key); // ?
		//const std::vector<Location>::iterator getLocationKey(std::string key); // ?

		void checkSemiColon(std::string &input);
		bool checkDupLocations(void);
		void updateLocations(void);
		std::string intToString(long long number);
		public:
		class ErrorException : public std::exception
		{
			private:
				std::string _message;
			public:
				ErrorException(std::string message) throw()
				{
					_message = "SERVER CONFIG ERROR: " + message;
				}
				virtual const char* what() const throw()
				{
					return (_message.c_str());
				}
				virtual ~ErrorException() throw() {}
		};

	friend class Handler;
};

#endif

