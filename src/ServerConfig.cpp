/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/19 19:12:28 by bmirlico          #+#    #+#             */
/*   Updated: 2024/06/24 20:11:43 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

ServerConfig::ServerConfig(void)
{
	this->_port = 0;
	this->_host = "";
	this->_serverName = "";
	this->_root = "";
	this->_clientMaxBodySize = MAX_BODY_LENGTH;
	this->_index = "";
	this->_autoIndex = false;
	this->initErrorPages();
}

ServerConfig::~ServerConfig() {}

ServerConfig::ServerConfig(const ServerConfig &cpy)
{
	*this = cpy;
}

ServerConfig &ServerConfig::operator=(const ServerConfig & rhs)
{
	if (this != &rhs)
	{
		this->_serverName = rhs._serverName;
		this->_root = rhs._root;
		this->_port = rhs._port;
		this->_host = rhs._host;
		this->_clientMaxBodySize = rhs._clientMaxBodySize;
		this->_index = rhs._index;
		this->_errorPages = rhs._errorPages;
		this->_locations = rhs._locations;
		this->_autoIndex = rhs._autoIndex;
	}
	return (*this);
}

void ServerConfig::initErrorPages(void)
{
	// _errorPages[301] = "";
	// _errorPages[302] = "";
	// _errorPages[400] = "";
	// _errorPages[401] = "";
	// _errorPages[402] = "";
	// _errorPages[403] = "";
	// _errorPages[404] = "";
	// _errorPages[405] = "";
	// _errorPages[406] = "";
	// _errorPages[500] = "";
	// _errorPages[501] = "";
	// _errorPages[502] = "";
	// _errorPages[503] = "";
	// _errorPages[504] = "";
	// _errorPages[505] = "";
}

void ServerConfig::checkSemiColon(std::string &input)
{
	size_t pos = input.rfind(';');
	if (pos != input.size() - 1)
		throw ErrorException("Invalid format: each directive should be finished with ';' .");
	input.erase(pos);
}

void ServerConfig::setServerName(std::string serverName)
{
	checkSemiColon(serverName);
	this->_serverName = serverName;
}

void ServerConfig::setHost(std::string host)
{
	checkSemiColon(host);
	if (host == "localhost")
		host = "127.0.0.1";
	if (!isValidHost(host))
		throw ErrorException("Wrong syntax: host.");
	this->_host = host;
}

bool ServerConfig::isInteger(std::string& s)
{
	if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+')))
		return (false);
    char* p;
    std::strtol(s.c_str(), &p, 10);
    return (*p == 0);
}

bool ServerConfig::isValidHost(std::string host)
{
	std::istringstream ss(host);
    std::string token;
    int segments = 0;

    while (std::getline(ss, token, '.'))
	{
        if (!isInteger(token))
			return (false);
        int num = std::atoi(token.c_str());
        if (num < 0 || num > 255)
			return (false);
        segments++;
    }
    return (segments == 4);
}

void ServerConfig::setRoot(std::string root)
{
	checkSemiColon(root);
	this->_root = root;
}

void ServerConfig::setPort(std::string input)
{
	unsigned int port;

	port = 0;
	checkSemiColon(input);
	for (size_t i = 0; i < input.length(); i++)
	{
		if (!std::isdigit(input[i]))
			throw ErrorException("Port can only be an integer.");
	}
	port = std::strtol(input.c_str(), NULL, 10);
	if (port > 65635)
		throw ErrorException("Invalid port value, too high.");
	else if (port < 1024)
		throw ErrorException("Invalid port value, permission denied.");
	this->_port = (uint16_t)port;
}

void ServerConfig::setClientMaxBodySize(std::string input)
{
	long long body_size;

	body_size = 0;
	errno = 0;
	checkSemiColon(input);
	for (size_t i = 0; i < input.length(); i++)
	{
		if (!(std::isdigit(input[i])))
			throw ErrorException("Client_max_body_size can only be a positive integer.");
	}
	body_size = std::strtoll(input.c_str(), NULL, 10);
	if (errno == ERANGE && body_size == LLONG_MAX)
		throw ErrorException("Invalid client_max_body_size value.");
	this->_clientMaxBodySize = body_size;
}

void ServerConfig::setIndex(std::string index)
{
	checkSemiColon(index);
	this->_index = index;
}

void ServerConfig::setAutoindex(std::string autoindex)
{
	checkSemiColon(autoindex);
	if (autoindex != "on" && autoindex != "off")
		throw ErrorException("Autoindex can only take on or off as values.");
	if (autoindex == "on")
		this->_autoIndex = true;
}

void ServerConfig::setErrorPages(std::vector<std::string> &errorPages)
{
	if (errorPages.empty())
		return ;
	if (errorPages.size() < 2) // le vector error_page doit au moins contenir un code d'error et une page d'erreur
		throw ErrorException ("Error page initialization failed.");
	std::string errorPagePath = errorPages.back();
	checkSemiColon(errorPagePath);
	for (size_t i = 0; i < errorPages.size() - 1; i++)
	{
		for (size_t j = 0; j < errorPages[i].size(); j++)
		{
			if (!std::isdigit(errorPages[i][j]))
				throw ErrorException("Error code should only contain digits.");
		}
		int codeError = std::strtol(errorPages[i].c_str(), NULL, 10);
		if (codeError < 300 || codeError > 599)
			throw ErrorException ("Incorrect error code: " + errorPages[i]);
		std::map<int, std::string>::iterator it = this->_errorPages.find(codeError);
		if (it == this->_errorPages.end())
			this->_errorPages.insert(std::make_pair(codeError, errorPagePath));
		else
			throw ErrorException("Error code is duplicated."); // j'ai décidé qu'on ne pouvait pas avoir 2x le même error code pour faciliter le parsing
	}
}

std::string ServerConfig::statusCodeString(int statusCode)
{
    switch (statusCode)
    {
        case 100:
            return "Continue";
        case 101:
            return "Switching Protocol";
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 202:
            return "Accepted";
        case 203:
            return "Non-Authoritative Information";
        case 204:
            return "No Content";
        case 205:
            return "Reset Content";
        case 206:
            return "Partial Content";
        case 300:
            return "Multiple Choice";
        case 301:
            return "Moved Permanently";
        case 302:
            return "Moved Temporarily";
        case 303:
            return "See Other";
        case 304:
            return "Not Modified";
        case 307:
            return "Temporary Redirect";
        case 308:
            return "Permanent Redirect";
        case 400:
            return "Bad Request";
        case 401:
            return "Unauthorized";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        case 406:
            return "Not Acceptable";
        case 407:
            return "Proxy Authentication Required";
        case 408:
            return "Request Timeout";
        case 409:
            return "Conflict";
        case 410:
            return "Gone";
        case 411:
            return "Length Required";
        case 412:
            return "Precondition Failed";
        case 413:
            return "Payload Too Large";
        case 414:
            return "URI Too Long";
        case 415:
            return "Unsupported Media Type";
        case 416:
            return "Requested Range Not Satisfiable";
        case 417:
            return "Expectation Failed";
        case 418:
            return "I'm a teapot";
        case 421:
            return "Misdirected Request";
        case 425:
            return "Too Early";
        case 426:
            return "Upgrade Required";
        case 428:
            return "Precondition Required";
        case 429:
            return "Too Many Requests";
        case 431:
            return "Request Header Fields Too Large";
        case 451:
            return "Unavailable for Legal Reasons";
        case 500:
            return "Internal Server Error";
        case 501:
            return "Not Implemented";
        case 502:
            return "Bad Gateway";
        case 503:
            return "Service Unavailable";
        case 504:
            return "Gateway Timeout";
        case 505:
            return "HTTP Version Not Supported";
        case 506:
            return "Variant Also Negotiates";
        case 507:
            return "Insufficient Storage";
        case 510:
            return "Not Extended";
        case 511:
            return "Network Authentication Required";
        default:
            return "Undefined";
        }
}

bool ServerConfig::isValidErrorPages(void)
{
	std::map<int, std::string>::const_iterator it;
	for (it = this->_errorPages.begin(); it != this->_errorPages.end(); it++)
	{
		if (it->first < 100 || it->first > 599)
			return (false);
	}
	return (true);
}

std::string ServerConfig::intToString(long long number)
{
    std::ostringstream oss;
    oss << number;
    return oss.str();
}

void ServerConfig::setLocation(std::string path, std::vector<std::string> input)
{
	Location newLocation;
	std::vector<std::string> methods;
	bool flag_methods = false;
	bool flagAutoIndex = false;
	bool flagMaxSize = false;

	newLocation.setPath(path);
	for (size_t i = 0; i < input.size(); i++)
	{
		if (input[i] == "root" && (i + 1) < input.size())
		{
			if (!newLocation.getRootLocation().empty()) // vérifie si le root du block location est dupliqué ou non
				throw ErrorException("Root of location is duplicated.");
			if (!newLocation.getAlias().empty()) // vérifie s'il y a une directive alias en plus de root, et si tel est le cas renvoie une error
				throw ErrorException("Root and alias can't be defined in the same location block.");
			checkSemiColon(input[++i]);
			newLocation.setRootLocation(input[i]);
		}
		else if ((input[i] == "allow_methods") && (i + 1) < input.size())
		{
			if (flag_methods)
				throw ErrorException("Allow_methods of location block is duplicated.");
			std::vector<std::string> methods;
			while (++i < input.size())
			{
				if (input[i].find(";") != std::string::npos)
				{
					checkSemiColon(input[i]);
					methods.push_back(input[i]);
					break ;
				}
				else
				{
					methods.push_back(input[i]);
					if (i + 1 >= input.size())
						throw ErrorException("Invalid method, out of location scope.");
				}
			}
			newLocation.setMethods(methods);
			flag_methods = true;
		}
		else if (input[i] == "autoindex" && (i + 1) < input.size())
		{
			if (path == "/cgi-bin")
				throw ErrorException("Autoindex not allowed for CGI location block.");
			if (flagAutoIndex)
				throw ErrorException("Autoindex of location is duplicated.");
			checkSemiColon(input[++i]);
			newLocation.setAutoindex(input[i]);
			flagAutoIndex = true;
		}
		else if (input[i] == "index" && (i + 1) < input.size())
		{
			if (!newLocation.getIndexLocation().empty())
				throw ErrorException("Index of location is duplicated.");
			checkSemiColon(input[++i]);
			newLocation.setIndexLocation(input[i]);
		}
		else if (input[i] == "upload" && (i + 1) < input.size())
		{
			if (!newLocation.getUploadLocation().empty())
				throw ErrorException("Upload of location is duplicated.");
			checkSemiColon(input[++i]);
			newLocation.setUploadLocation(input[i]);
		}
		else if (input[i] == "return" && (i + 1) < input.size())
		{
			if (path == "/cgi-bin")
				throw ErrorException("Return not allowed for CGI location block.");
			if (!newLocation.getReturn().empty())
				throw ErrorException("Return of location block is duplicated.");
			std::vector<std::string> redirs;
			while (++i < input.size())
			{
				if (input[i].find(";") != std::string::npos)
				{
					checkSemiColon(input[i]);
					redirs.push_back(input[i]);
					break ;
				}
				else
				{
					redirs.push_back(input[i]);
					if (i + 1 >= input.size())
						throw ErrorException("Invalid redirection, out of location scope.");
				}
			}
			newLocation.setReturn(redirs);
		}
		else if (input[i] == "alias" && (i + 1) < input.size())
		{
			if (!newLocation.getRootLocation().empty())
				throw ErrorException("Root and alias can't be defined in the same location block.");
			if (path == "/cgi-bin")
				throw ErrorException("Alias not allowed for CGI");
			if (!newLocation.getAlias().empty())
				throw ErrorException("Alias of location block is duplicated.");
			checkSemiColon(input[++i]);
			newLocation.setAlias(input[i]);
		}
		else if (input[i] == "cgi_ext" && (i + 1) < input.size())
		{
			if (path != "/cgi-bin")
				throw ErrorException("Cgi_ext not allowed for non CGI location block.");
			std::string extension;
			if (input[++i].find(";") != std::string::npos)
			{
				checkSemiColon(input[i]);
				extension = input[i];
			}
			else
				throw ErrorException("Only one CGI extension required.");
			if (extension != ".py")
				throw ErrorException("Invalid CGI extension, must be '.py'.");
			newLocation.setCgiExtension(extension);
		}
		else if (input[i] == "cgi_interpreter" && (i + 1) < input.size())
		{
			if (path != "/cgi-bin")
				throw ErrorException("Cgi_interpreter not allowed for non CGI location block.");
			std::string path;
			if (input[++i].find(";") != std::string::npos)
			{
				checkSemiColon(input[i]);
				path = input[i];
			}
			else
				throw ErrorException("Only one CGI interpreter required.");
			if (input[i].find("/python") == std::string::npos)
				throw ErrorException("cgi_interpreter path is invalid, must contain '/python'.");
			newLocation.setCgiInterpreter(path);
		}
		else if (input[i] == "client_max_body_size" && (i + 1) < input.size())
		{
			if (flagMaxSize)
				throw ErrorException("Maxbody_size of location is duplicated");
			checkSemiColon(input[++i]);
			newLocation.setMaxBodySizeLoc(input[i]);
			flagMaxSize= true;
		}
		else if (i < input.size())
		{
			std::cout << input[i] << std::endl;
			throw ErrorException("Unsupported directive in location block."); // condition qui vérifie si on nous envoie n'importe quoi dans le block location et renvoie une error
		}
	}
	std::vector<std::string> methodsDefault;
	if (!flag_methods)
		newLocation.setMethods(methodsDefault); // met les méthodes du CGI par défaut GET, POST et la méthode GET par défaut pour les blocs location classique
	this->_locations.push_back(newLocation);
}

// fonction qui check si 2 blocks location sont identiques !
bool ServerConfig::checkDupLocations(void)
{
	if (this->_locations.size() < 2)
		return (false);
	std::vector<Location>::const_iterator it1;
	std::vector<Location>::const_iterator it2;
	for (it1 = this->_locations.begin(); it1 != this->_locations.end() - 1; it1++) {
		for (it2 = it1 + 1; it2 != this->_locations.end(); it2++) {
			if (it1->getPath() == it2->getPath())
				return (true);
		}
	}
	return (false);
}

// fonction qui permet de mettre par défaut les directives des blocks location à celles du block server
// ex: si un block location n'a pas de root, body size ou index => on lui assigne ceux du block server
void ServerConfig::updateLocations()
{
	for (size_t i = 0; i < this->_locations.size(); i++)
	{
		if (!this->_root.empty() && this->_locations[i].getRootLocation().empty()) // s'il n'y pas de root dans le block location on met le root du server
			this->_locations[i].setRootLocation(this->_root);
		if (this->_clientMaxBodySize != MAX_BODY_LENGTH && this->_locations[i].getMaxBodySizeLoc() == MAX_BODY_LENGTH)
			this->_locations[i].setMaxBodySizeLoc(intToString(this->_clientMaxBodySize));
		if (!this->_index.empty() && this->_locations[i].getIndexLocation().empty())
			this->_locations[i].setIndexLocation(this->_index);
	}
}

// Getters

const std::string &ServerConfig::getServerName()
{
	return (this->_serverName);
}

const std::string &ServerConfig::getRoot()
{
	return (this->_root);
}

const bool &ServerConfig::getAutoindex()
{
	return (this->_autoIndex);
}

const std::string &ServerConfig::getHost()
{
	return (this->_host);
}

const uint16_t &ServerConfig::getPort()
{
	return (this->_port);
}

const size_t &ServerConfig::getClientMaxBodySize()
{
	return (this->_clientMaxBodySize);
}

const std::vector<Location> &ServerConfig::getLocations()
{
	return (this->_locations);
}

const std::map<int, std::string> &ServerConfig::getErrorPages()
{
	return (this->_errorPages);
}

const std::string &ServerConfig::getIndex()
{
	return (this->_index);
}
