/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/19 23:26:05 by bmirlico          #+#    #+#             */
/*   Updated: 2024/06/24 17:33:31 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "ConfigFile.hpp"

class Location
{
	private:
		std::string					_pathLoc; // la route du block
		std::string					_rootLoc; // le root du block
		bool						_autoIndexLoc;
		std::string					_indexLoc; // l'index file du block
		std::string					_uploadLoc;
		std::vector<int>			_methods; // GET + POST + DELETE
		std::vector<std::string>			_return;
		std::string					_alias;
		std::string					_cgiInterpreter; // le path de l'interpreters utilisé pour les scripts CGI (par ex /usr/python3 pour python)
		std::string					_cgiExt; // l'extension du block cgi (.py par ex si on choisit de faire un CGI en python)
		unsigned long				_clientMaxBodySizeLoc;

	public:
		std::map<std::string, std::string> _extensions;

		Location(void);
		Location(const Location &cpy);
		Location &operator=(const Location &rhs);
		~Location(void);

		void setPath(std::string pathLoc);
		void setRootLocation(std::string rootLoc);
		void setMethods(std::vector<std::string> methods);
		void setAutoindex(std::string autoIndexLoc);
		void setIndexLocation(std::string indexLoc);
		void setUploadLocation(std::string uploadLoc);
		void setReturn(std::vector<std::string> input);
		void setAlias(std::string input);
		void setCgiInterpreter(std::string path);
		void setCgiExtension(std::string extension);
		void setMaxBodySizeLoc(std::string input);

		const std::string &getPath() const;
		const std::string &getRootLocation() const;
		std::vector<int> &getMethods();
		const bool &getAutoIndexLoc() const;
		const std::string &getIndexLocation() const;
		const std::string &getUploadLocation() const;
		const std::vector<std::string> &getReturn() const;
		const std::string &getAlias() const;
		const std::string &getCgiInterpreter() const;
		const std::string &getCgiExtension() const;
		const std::map<std::string, std::string> &getExtensionPath() const;
		const unsigned long &getMaxBodySizeLoc() const;
		std::string getPrintMethods(void) const;
		public:
		class ErrorException : public std::exception
		{
			private:
				std::string _message;
			public:
				ErrorException(std::string message) throw()
				{
					_message = "LOCATION CONFIG ERROR: " + message;
				}
				virtual const char* what() const throw()
				{
					return (_message.c_str());
				}
				virtual ~ErrorException() throw() {}
		};

		friend class Handler;
		friend class Request;
		// friend class Connection;
		// friend class Response;
};

#endif
