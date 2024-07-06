/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/13 12:29:28 by clbernar          #+#    #+#             */
/*   Updated: 2024/07/06 15:25:57 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "Handler.hpp"

Request::Request() : m_ready(false), m_chunked(false), m_end_of_chunked(false), m_uriIsADirectory(false), m_body_pos(0), m_error_code(0), m_response_code(0), m_headers(), m_cgi(false), m_redirection(false)
{
	// std::cout<<"Request constructor called"<<std::endl;
}

Request::Request(Request const& asign)
{
	m_read = asign.m_read;
	m_chunked_body = asign.m_chunked_body;
	m_ready = asign.m_ready;
	m_chunked = asign.m_chunked;
	m_end_of_chunked = asign.m_end_of_chunked;
	m_method = asign.m_method;
	m_uri = asign.m_uri;
	m_uriIsADirectory = asign.m_uriIsADirectory;
	m_query = asign.m_query;
	m_protocol = asign.m_protocol;
	m_body_pos = asign.m_body_pos;
	m_error_code = asign.m_error_code;
	m_response_code = asign.m_response_code;
	m_headers = asign.m_headers;
	m_cgi = asign.m_cgi;
	m_filename = asign.m_filename;
	m_pipe = asign.m_pipe;
	m_redirection = asign.m_redirection;
	// std::cout<<"Request copy constructor called"<<std::endl;
}

Request::~Request()
{
	// std::cout<<"Request destructor called"<<std::endl;
}

Request& Request::operator=(Request const & equal)
{
	// std::cout<<"Request copy operator called"<<std::endl;
	if (this != &equal)
	{
		m_read = equal.m_read;
		m_chunked_body = equal.m_chunked_body;
		m_ready = equal.m_ready;
		m_chunked = equal.m_chunked;
		m_end_of_chunked = equal.m_end_of_chunked;
		m_method = equal.m_method;
		m_uri = equal.m_uri;
		m_uriIsADirectory = equal.m_uriIsADirectory;
		m_query = equal.m_query;
		m_protocol = equal.m_protocol;
		m_body_pos = equal.m_body_pos;
		m_error_code = equal.m_error_code;
		m_response_code = equal.m_response_code;
		m_headers = equal.m_headers;
		m_cgi = equal.m_cgi;
		m_filename = equal.m_filename;
		m_pipe = equal.m_pipe;
		m_redirection = equal.m_redirection;
	}
	return *this;
}

/***********************************************************************************************************
 *                                                                                                         *
 *                                              PARSING REQUEST                                            *
 *                                                                                                         *
 ***********************************************************************************************************/

void	Request::parseRequest()
{
	unsigned char	endOfHeaders[] = {'\r', '\n', '\r', '\n'};
	std::vector<unsigned char>::iterator	it;
	it = std::search(this->m_read.begin(), this->m_read.end(), endOfHeaders, endOfHeaders + 4);
	if (it != this->m_read.end())
	{
		if (m_body_pos == 0)// Pour pas le mettre a a jour a chaque fois / Je ne le mets pas direct a + 4car je ne sais pas si y'a vraiment un Body
			m_body_pos = it - m_read.begin();
		// std::cout<<"START PARSING"<<std::endl;
		size_t startHeaders = parseRequestLine();
		parseHeaders(startHeaders);
		handleChunked();
		if (!m_chunked)
		{
			mapString::iterator it = m_headers.find("Content-Length:");
			if (it == m_headers.end())// Pour passer outre le if else if dans la main loop
			{
				// PRINT_RED("TEST1")<<std::endl;
				m_ready = true;
			}
			else
			{
				if (m_read.size() > m_body_pos + 4 && m_read.size() - (m_body_pos + 4) >= (unsigned long)std::atoi(m_headers["Content-Length:"].c_str()))
				{
					// PRINT_RED("TEST2 taille du body tester = ")<<m_read.size() - (m_body_pos + 4)<<" taille de Content-Length teste = "<<(unsigned long)std::atoi(m_headers["Content-Length:"].c_str())<<std::endl;
					m_ready = true;
				}
			}
		}
		else if (m_end_of_chunked)
		{
			m_ready = true;
			// PRINT_RED("TEST3")<<std::endl;
		}
	}
	// else
		// std::cout<<"La requete n'as pas encore ete lue jusqu'a la fin des Headers"<<std::endl;
}

// This function parse the RequestLine (the first one)
// A good RequestLine request 3 elements [Method URI Protocol]
size_t	Request::parseRequestLine()
{
	if (m_uri.size() != 0)
		return 0;
	std::string	requestLine(m_read.begin(), m_read.end());
	size_t	pos = requestLine.find("\r\n");
	// std::cout<<"Affichage de pos = ["<<m_read[pos + 2]<<"]"<<std::endl;
	if (pos != std::string::npos)
	{
		std::string	line = requestLine.substr(0, pos);
		size_t	space1 = line.find(' ');
		size_t	space2 = line.find(' ', space1 + 1);
		// CHECK MORE THAN 3 ELEMENTS
		size_t	space3 = line.find(' ', space2 + 1);
		if (space3 != std::string::npos)
		{
			m_error_code = 400;
			// PRINT_RED("Mauvais format dans parseRequestLine")<<std::endl;
		}// CHECK LESS THAN 3 ELEMENTS
		else if (space1 != std::string::npos && space2 != std::string::npos)
		{
			m_method = line.substr(0, space1);
			std::string uri = line.substr(space1 + 1, space2 - space1 - 1);
			m_protocol = line.substr(space2 + 1);
			if (!MethodAllowed(m_method))
			{
				// PRINT_RED("Method Not Allowed")<<std::endl;
				m_error_code = 405;
			}
			else if (!UriValid(uri))
				return 0;
			else if (!ProtocolValid(m_protocol))
			{
				// PRINT_RED("Protocol Not Supported")<<std::endl;
				m_error_code = 505;
			}
		}
		else
		{
			m_error_code = 400;
			// PRINT_RED("Mauvais format dans parseRequestLine")<<std::endl;
		}
	}
	else
	{
		m_error_code = 400;
		// PRINT_RED("Mauvais format dans parseRequestLine")<<std::endl;
	}
	return pos + 2;// Pour conserver la requete originale mais une fois que tout est bon modifier en supprimant ce qui a ete parser pour garder uniquement le body
}

bool	Request::MethodAllowed(std::string const & method) const
{
	return method == "GET" || method == "POST" || method == "DELETE";
}

bool	Request::ProtocolValid(std::string const & protocol) const
{
	return protocol == "HTTP/1.1";
}

// Certains caracteres dit reserves (cf RFC) doivent etre interpretes ou non dans certains cas
// Pour un serveur simple comme webserv, j'autorise l'ensemble de ces caracteres que je n'interpreterais pas
// L'utilisation de ces caracteres devra donc forcement correspondre a une ressource existante pour ne pas
// engendrer d'erreur.
// RFC 3986 concernant les URI
bool	Request::UriValid(std::string & uri)
{
	// URI SIZE
	if (uri.size() > URI_SIZE_MAX)
	{
		m_error_code = 414;
		// PRINT_RED("URI is too large")<<std::endl;
		return false;
	}
	// Split URI & Query
	size_t pos = uri.find('?');
	if (pos != std::string::npos)
	{
		m_uri = uri.substr(0, pos);
		m_query = uri.substr(pos + 1);
	}
	else
		m_uri = uri;
	// URI CONTENT
	// for (size_t i = 0; i < uri.length(); ++i)
	// {
	// 	char c = uri[i];
	// 	if (!isalnum(c) && (c != '-' || c != '.' || c != '_' || c != '~' || c != '/'))
	// 	{
	// 		PRINT_RED("URI is invalid")<<std::endl;
	// 		m_error_code = 400;
	// 		return false;
	// 	}
	// }
	return true;
}

// This functions checks size limit for headers for security
// It splits headers line by line and pass each line to HeaderLineValid for parsing
void	Request::parseHeaders(size_t startHeaders)
{
	if (m_error_code != 0 || m_headers.size() != 0)// Check Error Parsing RequestLine
		return;
	std::string	Headers(m_read.begin() + startHeaders, m_read.end());
	// CHECK DE LA SIZE DE L'ENSEMBLE DES HEADERS
	if (Headers.size() == 0)
	{
		m_error_code = 400;
		// PRINT_RED("WARNING : No Headers, Suspicious activity")<<std::endl;
		return ;
	}
	else if (Headers.size() > HEADERTOTAL_MAX_SIZE)
	{
		m_error_code = 431;
		// PRINT_RED("Request Headers Fields Too Large")<<std::endl;
		return ;
	}
	size_t initial_pos = 0;
	size_t pos = 0;
	// std::cout<<"HEADERS SIZE == "<<Headers.size()<<std::endl;
	// std::cout<<"HEADERS LAST CHAR == "<<Headers[424]<<Headers[425]<<Headers[426]<<Headers[427]<<std::endl;
	// std::cout<<"BODY POS == "<<m_body_pos<<std::endl;
	// std::cout<<"BODY CONTENT 4 last char == "<<m_read[m_body_pos + 3]<<std::endl;
	// SPLIT LIGNE PAR LIGNE DE CHAQUE HEADER
	while ((pos + startHeaders) != m_body_pos)
	{
		pos = Headers.find("\r\n", initial_pos + 1);
		// std::cout<<"Initial pos == "<<initial_pos<<" // pos == "<<pos<<std::endl;
		std::string header = Headers.substr(initial_pos, pos - initial_pos);
		if (!HeaderLineValid(header))
			break ;
		initial_pos = pos + 2;
	}
	mapString::iterator it = m_headers.find("Host:");
	if (it == m_headers.end())
		m_error_code = 400;
	// m_body_pos est ok = Si il y a un Body, il start a m_body_pos + 4 [A CHECKER]
	// PRINT_GREEN("Resultat de fin de parsing des Headers :")<<std::endl;
	// mapString::iterator it;
	// for (it = m_headers.begin(); it != m_headers.end(); ++it)
	// {
	// 	std::cout<<it->first<<" "<<it->second<<std::endl;
	// }
}

// This function checks HeaderLine's validity. Checks are size limit,and good syntax [key: value]
// As NGINX, webserv will be very permissive about the content. A bad header content won't be interpreted, that's it.
bool	Request::HeaderLineValid(std::string &header)
{
	// std::cout<<"Ma string contenant un header == "<<header<<std::endl;
	// CHECK DE LA SIZE DE LA HEADERLINE
	if (header.size() > HEADERLINE_MAX_SIZE)
	{
		m_error_code = 431;
		// PRINT_RED("Request Fields Too Large")<<std::endl;
		return false;
	}
	// SPLIT DE HEADERLINE POUR CHECKER LE FORMAT [key: value]
	size_t	space = header.find(' ');
	if (space == std::string::npos)// NO SPACE ON HEADERLINE
	{
		// std::cout<<"No space in header line => Wrong syntax"<<std::endl;
		m_error_code = 400;
		return false;
	}
	else//CHECK DE LA SYNTAXE [key: value]
	{
		std::string key = header.substr(0, space);
		if (key[key.size() - 1] != ':')// PAS DE ":" A A FIN DE KEY dans [key: value]
		{
			m_error_code = 400;
			// PRINT_RED("PAS LA BONNE SYNTAXE")<<" Last element de key: == "<<key[key.size() - 1]<<std::endl;
			return false;
		}
		// else
		// 	PRINT_GREEN("HEADER A LA BONNE SYNTAXE")<<std::endl;
		std::string value = header.substr(space + 1);
		// std::cout<<"Format [key:][value] == ["<<key<<"]["<<value<<"]"<<std::endl;
		// STOCKAGE DU HEADER
		m_headers[key] = value;
		if (key == "Transfer-Encoding:" && value == "chunked")
		{
			// PRINT_GREEN("La requete est chunked")<<std::endl;
			m_chunked = true;
		}
	}
	return true;
}

// This function checks the validity of Content-Length's value
bool	Request::checkContentLengthValue(std::string & value, long long &content_length)
{
	char *end;
	try
	{
		content_length = strtoll(value.c_str(), &end, 10);
		if (end == value.c_str() || *end != '\0')
			throw std::invalid_argument("Invalid Content-Length value");
		if (content_length < 0 || content_length > INT_MAX)
			throw std::out_of_range("Content-Length value out of range");
		if (content_length == 0)
		{
			// PRINT_RED("Content-Length = 0 dans checkContentValue")<<std::endl;
			m_error_code = 400;
			return false;
		}
	}
	catch (const std::exception& e)
	{
		PRINT_RED(e.what())<<std::endl;
		m_error_code = 400;
		return false;
	}
	return true;
}

// Once the corresponding config is found, this function checks if the body and the method
// received respect config. It prepares the uri received depending on the config
void	Request::setConfig(ServerConfig & serverBlock, int loc_index)
{
	// Set chemin resolu
	if (loc_index != -1 && !serverBlock._locations[loc_index]._alias.empty())// Alias donc pas de root
	{
		// Remplacer les elements egaux a _pathLoc par la valeur de l'alias
		m_uri.erase(0, serverBlock._locations[loc_index]._pathLoc.size());
		m_uri.insert(0, serverBlock._locations[loc_index]._alias);
		// PRINT_GREEN("TEST de m_uri apres alias : ")<<m_uri<<std::endl;
	}
	else // Pas d'alias donc set de root
	{
		if (loc_index != -1 && !serverBlock._locations[loc_index]._rootLoc.empty())
		{
			// std::cout<<"rentre dans le if"<<std::endl;
			m_root = serverBlock._locations[loc_index]._rootLoc;
			// m_uri.erase(0, serverBlock._locations[loc_index]._pathLoc.size());
		}
		else
		{
			// std::cout<<"rentre dans le else"<<std::endl;
			m_root = serverBlock._root;
		}
		// PRINT_GREEN("TEST de m_root : ")<<m_root<<std::endl;
	}
	if (m_root.size() != 0 && m_root[m_root.size() - 1] != '/' && (loc_index != -1 && serverBlock._locations[loc_index]._cgiExt.empty()))
		m_root += "/";
	if (m_uri.size() > 0 && m_uri[0] == '/')
		m_uri.erase(m_uri.begin());
	m_uri = m_root + m_uri;
	// std::cout<<"TEST de m_uri : "<<m_uri<<std::endl;
	// Methodes allowed a faire !
	if ((m_method == "GET" && loc_index != -1 && !serverBlock._locations[loc_index]._methods[0]) ||
		(m_method == "POST" && loc_index != -1 && !serverBlock._locations[loc_index]._methods[1]) ||
		(m_method == "DELETE" && loc_index != -1 && !serverBlock._locations[loc_index]._methods[2]))
	{
		// std::cout<<"La methode n'est pas autorisee"<<std::endl;
		m_error_code = 405;
		return;
	}
	// std::cout<<"La methode est autorisee"<<std::endl;
	// if (loc_index != -1)
	// {
	// 	std::cout<<"test valeur de get dans vecteur : "<<serverBlock._locations[loc_index]._methods[0]<<std::endl;
	// 	std::cout<<"test valeur de post dans vecteur : "<<serverBlock._locations[loc_index]._methods[1]<<std::endl;
	// 	std::cout<<"test valeur de delete dans vecteur : "<<serverBlock._locations[loc_index]._methods[2]<<std::endl;
	// }
	// CHECK BODY
	if (loc_index != -1 && serverBlock._locations[loc_index]._return.size() != 0)
		return ;
	unsigned long bodySizeMax = 0;
	if (serverBlock._clientMaxBodySize > 0)
		bodySizeMax = serverBlock._clientMaxBodySize;
	// PRINT_RED("TEST1 413 ")<<bodySizeMax<<std::endl;
	if (loc_index != -1 && serverBlock._locations[loc_index]._clientMaxBodySizeLoc > 0)
		bodySizeMax = serverBlock._locations[loc_index]._clientMaxBodySizeLoc;
	// PRINT_RED("TEST2 413 ")<<bodySizeMax<<std::endl;
	if (bodySizeMax != 0 && m_read.size() - m_body_pos > bodySizeMax)
	{
		// PRINT_RED("TEST3 413 ")<<bodySizeMax<<std::endl;
		m_error_code = 413;
		return ;
	}
}

// This function check if the server is waiting for a Body and it has been completely read
// To process Request
void	Request::checkBody()
{
	// PRINT_GREEN("checkbody called")<<std::endl;
	// std::cout<<"Body position element == "<<m_read[m_body_pos]<<std::endl;
	// std::cout<<" Body pos == "<<m_body_pos<<std::endl;
	if (m_method == "POST" && !m_chunked)
	{
		if (m_read.size() > m_body_pos + 4)
			m_body_pos += 4;
		mapString::iterator it = m_headers.find("Content-Length:");
		if (it != m_headers.end())// Si Content-Length est present pour POST
		{
			long long content_length = 0;
			// std::cout<<"Body = "<<m_read.size() - m_body_pos<<" pour un body de taille "<<it->second<<std::endl;
			if (!checkContentLengthValue(it->second, content_length)) //Si la valeur de Content-Length est invalide
				return ;
			// std::cout<<"readSize | BodyPos | contentLength == "<<m_read.size()<<"  "<<m_body_pos<<"  "<<content_length<<std::endl;
			if (m_read.size() - m_body_pos < (unsigned long)content_length)// Si Content-Length n'a pas ete lu en entier
			{
				m_error_code = 400;
				// PRINT_RED("Content-Length n'a pas ete lu en entier dnas CheckBody")<<std::endl;
			}
			else if (m_read.size() - m_body_pos > (unsigned long)content_length)// Si plus que Content-Length a ete lu
			{
				// PRINT_RED("TEST2 413")<<std::endl;
				m_error_code = 413;
			}
		}
		else// Si Content-Length n'est pas present pour POST
		{
			// PRINT_RED("Content-Length Pas prsent dans la requete");
			m_error_code = 411;// Length Required
		}
	}
}

// This function handle chunked request by extracting chunked body of different chunked part in a tmp vector<unsigned char> to use it as a normal Body
// While webserv stocks the received request, it parses the body. When one or more chunked are full (Size in hexadecimal : Chunked Body of defined size),
// It exctracts each chunked body until last chunked is received
void	Request::handleChunked()
{
	if (m_chunked && (m_read.size() > m_body_pos + 4) && m_error_code == 0)// Si c'est une request Chunked, qu'on a un Body et aucune erreur
	{
		// std::cout<<"\n\nTEST [Appel a la fonction handleChunked()]"<<std::endl;
		std::vector<unsigned char>::iterator bodyStart = m_read.begin() + m_body_pos + 4;
		const std::string crlf = "\r\n";
		int	pos = 0;
		while (1)
		{
			// std::cout<<"TEST [Debut de boucle] pos = "<<pos<<std::endl;
			if (m_end_of_chunked) // Cas ou on recoit encore alors qu'on a atteint deja le chunk de fin
			{
				m_error_code = 400;
				return ;
			}
			// CHECK LA PRESENCE D'UN HEXA POTENTIEL
			// std::cout<<"TEST [Pas de body en surplus] pos = "<<pos<<std::endl;
			std::vector<unsigned char>::iterator it = std::search(bodyStart + pos, m_read.end(), crlf.begin(), crlf.end());
			if (it == m_read.end())
			{
				m_read.erase(bodyStart, bodyStart + pos);
				break ;
			}
			// std::cout<<"TEST [verification dun CRLF de fin de hexa] pos = "<<pos<<std::endl;
			// CHECK VALIDITE L'HEXA
			std::string	hexaStr = std::string(bodyStart + pos, it);
			int	size = hexaToInt(hexaStr);
			// std::cout<<"Affichage de hexaStr = "<<hexaStr<<std::endl;
			// std::cout<<"Affichage de hexa convertit en int  = "<<size<<std::endl;
			if (size == -1)
			{
				m_end_of_chunked = true;
				return ;
			}
			// std::cout<<"TEST [size correcte] pos = "<<pos<<std::endl;
			pos += hexaStr.size() + 2;// Ajout du CRLF
			// CHECK SI LE CHUNK EST COMPLET
			if (std::distance(bodyStart + pos, m_read.end()) < size + 2)
			{
				m_read.erase(bodyStart, bodyStart + pos - (hexaStr.size() + 2));
				break;
			}
			// std::cout<<"TEST [Assez de taille pour un Content complet] pos = "<<pos<<std::endl;
			// CHECK DU FORMAT CRLF A LA FIN DU BODY CHUNKED
			if (bodyStart + pos + size != std::search(bodyStart + pos + size, m_read.end(), crlf.begin(), crlf.end()))
			{
				m_end_of_chunked = true;
				m_error_code = 400;
				return ;
			}
			// std::cout<<"TEST [Format CRLF apres le content ] pos = "<<pos<<std::endl;
			if (size == 0)
			{
				lastChunk(bodyStart, pos, size);
				return ;
			}
			// std::cout<<"TEST[Not lastChunk on insert dans chunked_body] pos = "<<pos<<std::endl;
			m_chunked_body.insert(m_chunked_body.end(), bodyStart + pos, bodyStart + pos + size);
			pos += size + 2;
		}
	}
}

// This function is called when last chunk is found. It clears read request body, put instead unchunked body
// and clear the vector that stored unchunked body
void	Request::lastChunk(std::vector<unsigned char>::iterator & bodyStart, int pos, int size)
{
	m_end_of_chunked = true;
	if (m_chunked_body.size() == 0)// Cas d'une requete Chunked vide (directement chunk vide)
	{
		m_error_code = 400;
		return ;
	}
	// std::cout<<"AFFICAHE DE CE QUI A ETE RECU ET TRAITER DANS lastCHUNK :"<<std::endl;
	// for (std::vector<unsigned char>::iterator print = bodyStart; print != bodyStart + pos + size + 2; ++print)// AFFICHAGE DE TEST
	// 	std::cout<<*print;
	m_read.erase(bodyStart, bodyStart + pos + size + 2);
	if (bodyStart != m_read.end())// On a de la donnee apres le chunk de fin
	{
		m_error_code = 400;
		return ;
	}
	m_read.insert(m_read.end(), m_chunked_body.begin(), m_chunked_body.end());
	m_chunked_body.erase(m_chunked_body.begin(), m_chunked_body.end());
	m_body_pos += 4;
	return ;
}

// This function convert a string representing an hexadecimal number in int
int	Request::hexaToInt(std::string const & toConvert)
{
	if (toConvert.empty())
	{
		// PRINT_RED("Empty string to Convert in hexadecimal")<<std::endl;
		m_error_code = 400;
		return -1;
	}
	char *end;
	long int	result = strtol(toConvert.c_str(), &end, 16);
	if (end == toConvert.c_str() || *end != '\0')
	{
		// PRINT_RED("Invalid digits found in string hexadecimal")<<std::endl;
		m_error_code = 400;
		return -1;
	}
	if (result > INT_MAX || result < 0)
	{
		// PRINT_RED("Hexa value out of rnge for int")<<std::endl;
		m_error_code = 400;
		return -1;
	}
	return static_cast<int>(result);
}

/***********************************************************************************************************
 *                                                                                                         *
 *                                              PROCESS REQUEST                                            *
 *                                                                                                         *
 ***********************************************************************************************************/

void	Request::processRequest(ServerConfig & m_config, int loc_index,  Response & response)
{
	// PRINT_GREEN("processRequest called")<<std::endl;
	if (m_error_code != 0 || (loc_index != -1 && m_config._locations[loc_index]._return.size() > 0))
		return ;
	if (loc_index != -1 && !m_config._locations[loc_index]._cgiInterpreter.empty())
		processCGI(m_config, loc_index);
	else if (m_method == "GET")
		processGet(m_config, loc_index, response);
	else if (m_method == "POST")
		processPost(m_config, loc_index, response);
	else if (m_method == "DELETE")
		processDelete(response);
}

bool Request::is_directory(const std::string& path)
{
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf) != 0)
		return false;
	return S_ISDIR(statbuf.st_mode);
}

void	Request::generateAutoIndex(const std::string& directory_path, Response & response)
{
	DIR* dir;
	struct dirent* ent;
	std::vector<std::string> entries;

	dir = opendir(directory_path.c_str());
	if (dir == NULL)
	{
		PRINT_RED("Could not open directory: ")<< directory_path << std::endl;
		m_error_code = 500;
		return ;
	}
	// Read directory entries
	while ((ent = readdir(dir)) != NULL) {
		entries.push_back(ent->d_name);
	}
	closedir(dir);
	std::vector<std::string>::iterator it = std::find(entries.begin(), entries.end(), ".");
	if (it != entries.end())
		entries.erase(it);
	std::vector<std::string>::iterator itcgi = std::find(entries.begin(), entries.end(), "cgi-bin");
	if (itcgi != entries.end())
		entries.erase(itcgi);
	// Find path without root to link well elements
	std::string uriWithoutRoot = m_uri;
	uriWithoutRoot.erase(0, m_root.size() - 1);
	if (uriWithoutRoot[uriWithoutRoot.size() - 1] != '/')
		uriWithoutRoot += '/';
	// Generate HTML
	std::string html = "<!DOCTYPE html>\r\n";
	html += "<html>\r\n\t<head>\r\n\t\t<title>Index of " + directory_path + "</title>\r\n\t</head>\r\n\t<body>" + "\r\n";
	html += "\t\t<h1>Index of " + directory_path + "</h1>\r\n\t\t<ul>" + "\r\n";
	for (size_t i = 0; i < entries.size(); ++i)
	{
		std::string entry = entries[i];
		std::string full_path = directory_path + "/" + entry;
		if (is_directory(full_path))
		{
			html += "\t\t\t<li><a href=\"" + uriWithoutRoot + entry + "/\">" + entry + "/</a></li>" + "\r\n";
		}
		else
		{
			html += "\t\t\t<li><a href=\"" + uriWithoutRoot + entry + "\">" + entry + "</a></li>" + "\r\n";
		}
	}
	html += "\t\t</ul>\r\n\t</body>\r\n</html>\r\n";
	response.m_response.insert(response.m_response.begin(), html.begin(), html.end());
	m_response_code = 200;
	m_uri += ".html";// to be conisederd as a html file by the client
}

bool	Request::processGetDirectory(Location & locationBlock, Response & response)
{
	(void)response;
	if (!locationBlock._indexLoc.empty())//Index
	{
		if (m_uri[m_uri.size() - 1] != '/')
			m_uri += "/";
		m_uri += locationBlock._indexLoc;
		if (!checkRessourceAccessibilty(m_uri))
			return false;
		// PRINT_RED("TEST de ressource en cas d'index : ")<<m_uri<<std::endl;
		return true;
	}
	else if (locationBlock._autoIndexLoc)//AutoIndex
	{
		// PRINT_RED("AutoIndex a faire [ BASTIEN ]")<<std::endl;
		// std::cout<<"Test autoIndex uri doit etre le repository : "<<m_uri<<std::endl;
		generateAutoIndex(m_uri, response);
		return false;
	}
	m_error_code = 403;
	return false;
}

// Attention Header Accept contenu ce qui est envoye ou renvoyer ??
// This function process a GET Request. If an error occured,it specifies it and stops
// If not,the ressource asked is placed in the Response object
void	Request::processGet(ServerConfig & config, int  loc_index, Response & response)
{
	// PRINT_GREEN("processGet Called")<<std::endl;
	if (loc_index != -1 && config._locations[loc_index]._return.size() != 0)
		return ;
	if (!checkRessourceAccessibilty(m_uri))
	{
		if (m_uriIsADirectory)
		{
			// std::cout<<"Test uriIsAD"<<std::endl;
			if (!processGetDirectory(config._locations[loc_index], response))
				return ;
		}
		else
			return ;
	}
	std::ifstream file(m_uri.c_str(), std::ios::binary);
	if (!file)
	{
		PRINT_RED("Impossible to open m_uri : Strange check processGet function ")<<m_uri<<std::endl;
		m_error_code = 500;// Checker d'autres cas possibles ?
		return ;
	}
	file.seekg(0, std::ios::end);
	std::streamsize fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	if (fileSize > MEGAOCTET)
		m_error_code = 413;
	else
	{
		// PRINT_RED("TEST Empty")<<std::endl;
		response.m_response.resize(fileSize);
		file.read(reinterpret_cast< char*>(&response.m_response[0]), fileSize);
		// response.m_response = std::vector<unsigned char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		response.m_body_size = response.m_response.size();
		m_response_code = 200;
	}
	file.close();
	if (response.m_response.size() != 0)
	{
		response.m_response.push_back('\r');
		response.m_response.push_back('\n');
	}
}

// This function checks the ressource asked in a request
// If it exists, is not a directory and has permission access,it returns true
bool	Request::checkRessourceAccessibilty(std::string const & ressource)
{
	struct stat s;
	// EXISTENCE
	// PRINT_RED("TEST de ressource : ")<<ressource<<std::endl;
	if (stat(ressource.c_str(), &s) != 0)// La ressource n'existe pas
	{
		PRINT_RED("Ressource doesn't exist : ")<<ressource<<std::endl;
		if (m_method != "POST")
			m_error_code = 404;
		return false;
	}
	// DIRECTORY
	if (s.st_mode & S_IFDIR)// A gerer avec la Config (Pour GET notamment)
	{
		PRINT_RED("Ressource is a directory : ")<<ressource<<std::endl;
		m_uriIsADirectory = true;
		if (m_method == "DELETE")
			m_error_code = 405;
		return false;
	}
	// PERMISSION
	if (m_method == "GET")
	{
		if (access(ressource.c_str(), R_OK) == -1)//Checker ordre : Check permission avant directory ?
		{
			PRINT_RED("Ressource doesn't have permission access : ")<<ressource<<std::endl;
			m_error_code = 403;
			return false;
		}
	}
	else if (m_method == "DELETE")
	{
		if (access(ressource.c_str(), R_OK | W_OK) == -1)//Checker ordre : Check permission avant directory ?
		{
			PRINT_RED("Ressource doesn't have permission access")<<std::endl;
			m_error_code = 403;
			return false;
		}
	}
	return true;
}

// This function process a Delete request. It checks ressource's accesibility and if the ressource
// can be deleted it does it, otherwise it prepares error_code to send error_file
void	Request::processDelete(Response & response)
{
	(void)response;
	// PRINT_GREEN("processDelete Called")<<std::endl;
	if (!checkRessourceAccessibilty(m_uri))
		return ;
	else// FILE
	{
		std::string body;
		// PRINT_GREEN("Ressource is a file")<<std::endl;
		if (std::remove(m_uri.c_str()) == 0)// DELETED
		{
			// body = "\r\nRessource has been deleted\r\n";
			m_response_code = 204;
		}
		else// NOT DELETED
		{
			// PRINT_RED("File could't be deleted")<<std::endl;
			m_error_code = 500;// ?? OU 403 Pa de permission si le parent n'a pas les permissions
		}
		// response.m_response.insert(response.m_response.end(), body.begin(), body.end());
		// response.m_content_type = "Content-Type: text/plain\r\n";
	}
}

// En cas de succes Message sous forme de text en tant que Body
// "Data received and processed successfully."|| "File uploaded successfully."
void	Request::processPost(ServerConfig & config, int loc_index, Response & response)
{
	(void)config;
	(void)response;
	// PRINT_GREEN("BODY RECEIVED SIZE : ")<<std::distance(m_read.begin() + m_body_pos, m_read.end())<<std::endl;
	// std::cout<<"\nRequest received :\n["<<std::endl;
	// for (std::vector<unsigned char>::iterator it = m_read.begin(); it != m_read.end(); ++it)// AFFICHAGE DE TEST
	// 		std::cout<<*it;
	// std::cout<<" ]"<<std::endl;
	// PRINT_GREEN("processPost Called")<<std::endl;
	std::string uploadLoc;
	if (loc_index != -1)
		uploadLoc = config._locations[loc_index]._uploadLoc;
	std::vector<unsigned char>	request_body(m_read.begin() + m_body_pos, m_read.end());
	mapString::iterator it = m_headers.find("Content-Type:");
	if (it != m_headers.end() && (it->second.find("multipart/form-data") != std::string::npos))
		processPostMultipart(extractBoundary(it->second), request_body, config, loc_index);
	else if (checkFileUpload()) // Televersement Simple A CHECKER !!!!!
		uploadFile(request_body, uploadLoc);
	else
		stockData(request_body);
	if (m_error_code == 0 && m_response_code == 201)// Cas multipart successfull
	{
		std::string body = "\r\nRessource has been created successfully\r\n";
		response.m_response.insert(response.m_response.end(), body.begin(), body.end());
		response.m_body_size = 43;
		response.m_content_type = "Content-Type: text/plain\r\n";
	}
}

// This function checks if a file has to be uploaded by checking the Content-Disposition header
// If it is present and contains a valid filename a file has to be uploaded
bool	Request::checkFileUpload()
{
	mapString::iterator it = m_headers.find("Content-Disposition:");
	if (it != m_headers.end())
	{
		extractFilename(it->second);
		if (m_filename.size() != 0)
			return true;
	}
	return false;
}

// This function store the body sent by the client in webserv's database
void	Request::stockData(std::vector<unsigned char> const & body)
{
	std::ofstream file("racine/DataBaseWebserv.txt", std::ios::binary | std::ios::app);
	if (!file)
	{
		PRINT_RED("Error : Imposible to open Webserv Data Base.")<<std::endl;
		m_error_code = 500;
		return ;
	}
	file.write(reinterpret_cast<const char*>(&body[0]), body.size());// Pb de perte de donnee avec du binaire ?
	if (!file)
	{
		PRINT_RED("Error writting Data Base")<<std::endl;
		m_error_code = 500;// ???
	}
	else if (body.size() != 0)
	{
		std::cout<<"Affichage de body size = "<<body.size()<<std::endl;
		m_response_code = 201;// A checker
		file << '\n' << '\n';
	}
	file.close();
}

// This function receive the Content-Disposition value and check if it contains a valid filename
// If so, it stores it in the request object
void	Request::extractFilename(std::string & toExtract)
{
	if (toExtract.find("filename=") == std::string::npos)
		return ;
	std::string filename = toExtract.substr(toExtract.find("filename=") + 9);
	// std::cout<<"filename = "<<filename;
	if (filename[0] == '"')
	{
		filename = filename.substr(1);
		// std::cout<<"filename2 = "<<filename;
		if (filename.find('"') == std::string::npos)
			return ;
		else
		{
			filename = filename.substr(0, filename.find('"'));
			// std::cout<<"filename3 = "<<filename;
			if (filename.size() <= 255 && filename.find("..") == std::string::npos
				&& filename.find("\\") == std::string::npos)
				m_filename = filename;
		}
	}
}

// This function check if the ressource webserv has to create exists and create it
// It fills it with body received in argument
void	Request::uploadFile(std::vector<unsigned char> &boundary_body, std::string & uploadLoc)
{
	// PRINT_GREEN("TEST de la taille de l'upload : ")<<boundary_body.size()<<std::endl;
	if (boundary_body.size() == 0 || m_filename.size() == 0)
		return ;
	if (boundary_body.size() > MEGAOCTET)
	{
		m_error_code = 413;
		return ;
	}
	std::string location;
	if (!uploadLoc.empty())
		location = m_root + uploadLoc + m_filename;
	else
		location = "racine/upload/" + m_filename;
	if	(checkRessourceAccessibilty(location))
	{
		m_error_code = 409;//Confilct exsite deja !
		return ;
	}
	else
	{
		std::ofstream file(location.c_str(), std::ios::binary);
		if (!file)
		{
			// PRINT_GREEN("TEST location = ")<<location<<std::endl;
			PRINT_RED("Error : Imposible to open Webserv Data Base.")<<std::endl;
			m_error_code = 500;
			return ;
		}
		file.write(reinterpret_cast<const char*>(&boundary_body[0]), boundary_body.size());// Pb de perte de donnee avec du binaire ?
		if (!file)
		{
			PRINT_RED("Error writting Data Base")<<std::endl;
			m_error_code = 500;
		}
		else if (boundary_body.size() != 0)
			m_response_code = 201;// A checker
		file.close();
	}
}

// This function is the main loop for processing each block of a multipart/form-data request
// It splits each block and sends it to processPostBoundaryBlock()
void	Request::processPostMultipart(std::string boundary, std::vector<unsigned char> & body, ServerConfig & config, int loc_index)
{
	if (boundary.size() == 0)
	{
		PRINT_RED("Boundarysize dans processMultipart")<<std::endl;
		m_error_code = 400;
		return ;
	}
	PRINT_GREEN(boundary.c_str())<<std::endl;
	std::string	end_boundary = boundary + "--";
	std::vector<unsigned char>::iterator it = std::search(body.begin(), body.end(), boundary.begin(), boundary.end());
	while (it != std::search(body.begin(), body.end(), end_boundary.begin(), end_boundary.end()))// Tant que le prochain boundary n'est pas le boundary de fin
	{
		if (it == body.begin())// EFFACE LE BOUNDARY DU DEBUT
		{
			// PRINT_RED("Debut boundary to cut")<<std::endl;
			body.erase(body.begin(), body.begin() + boundary.size() + 2); // supprime le boundary + "\r\n"
		}
		// else
		// 	PRINT_RED("Debut positon is not a boundary")<<std::endl;
		// PRINT_GREEN("Affichage de mon body tout au long de ma boucle")<<std::endl;
		// for (std::vector<unsigned char>::iterator print = body.begin(); print != body.end(); ++print)// Affichage jusqu'au prochain boundary
		// 	std::cout<<*print;
		// PRINT_GREEN("Affichage de mon block boundary")<<std::endl;//DECOUPE UN BLOCK
		std::vector<unsigned char>::iterator next = std::search(body.begin(), body.end(), boundary.begin(), boundary.end());
		// for (std::vector<unsigned char>::iterator test = body.begin(); test != next; ++test)// Affichage jusqu'au prochain boundary
		// 	std::cout<<*test;
		// TRAITEMENT DU BLOCK
		std::vector<unsigned char> boundary_block(it, next);
		processPostBoundaryBlock(boundary_block, config, loc_index);
		// EFFACE LE BLOCK DEJA TRAITE
		body.erase(body.begin(), next);// On efface le block qui a ete traite
		// if (it == std::search(body.begin(), body.end(), end_boundary.begin(), end_boundary.end()))// a remettre dans le while ??????!!!
		// 	break ;
	}
	if (m_error_code != 0 && m_response_code != 0)// Cas multiple Voir le comportement adequat 207 ??
	{
		// PRINT_GREEN("TEST multipart erro_code = ")<<m_error_code<<" response_code = "<<m_response_code<<std::endl;
		m_response_code = 0;
		m_error_code = 400;
	}
}

// This function recveive a multipart/form-data block, strores new headers from it thanks to updateHeaders()
// Depending on new headers result, it process the request by storing data or upload file
void	Request::processPostBoundaryBlock(std::vector<unsigned char> &boundary_block, ServerConfig & config, int loc_index)
{
	// PRINT
	// PRINT_GREEN("Traitement du block boundary")<<std::endl;
	// for (std::vector<unsigned char>::iterator it = boundary_block.begin(); it != boundary_block.end(); ++it)
	// 	std::cout<<*it;
	// SPLIT HEADERS / BODY
	std::string uploadLoc;
	if (loc_index != -1)
		uploadLoc = config._locations[loc_index]._uploadLoc;
	unsigned char	endOfHeaders[] = {'\r', '\n', '\r', '\n'};
	std::vector<unsigned char>::iterator	it;
	it = std::search(boundary_block.begin(), boundary_block.end(), endOfHeaders, endOfHeaders + 4);
	if (it == boundary_block.end())
	{
		PRINT_RED("Pas de fin de Headers trouve dans le block boundary")<<std::endl;
		m_error_code = 400;
		return ;
	}
	// STRING CONTENANT L'ENSEMBLE DES HEADERS
	// UPDATE m_headers
	std::string boundary_headers(boundary_block.begin(), it + 4);
	// std::cout<<"Affichage des boundary headers ::   "<<boundary_headers<<std::endl;
	if (it + 4 == boundary_block.end())// S'IL N'Y A PAS DE BODY
	{
		PRINT_RED("Pas de body dans processPostBoundaryBlock")<<std::endl;
		m_error_code = 400;
		return ;
	}
	std::vector<unsigned char> boundary_body(it + 4, boundary_block.end());
	// std::cout<<"Affichage du boundary body : "<<std::endl;
	// for (std::vector<unsigned char>::iterator itest = boundary_body.begin(); itest != boundary_body.end(); ++itest)
	// 	std::cout<<*itest;
	updateHeaders(boundary_headers);
	if (boundary_body.size() <= 2)
		return ;
	if (checkFileUpload())
	{
		// PRINT_RED("UPLOAD MULTIPART")<<std::endl;
		uploadFile(boundary_body, uploadLoc);//Reset multi part a inserer ici je pense
		resetMultipart();
	}
	else
	{
		// PRINT_RED("STOCK DATA MULTIPART")<<std::endl;
		stockData(boundary_body);
	}
}

// This function reset Content-Disposition header and filename between 2 boundary bloks
void	Request::resetMultipart()
{
	m_headers["Content-Disposition:"] = "";
	m_filename = "";
}

// This function update Headers with block multipart's header
void	Request::updateHeaders(std::string &boundary_headers)
{
	size_t start = 0;
	size_t end = boundary_headers.find("\r\n");
	while (end != (boundary_headers.size() - 2))
	{
		std::cout<< end + 2<<" == "<<boundary_headers.size()<<std::endl;
		std::string b_header = boundary_headers.substr(start, end - start);
		// std::cout<<"Affichage de ligne header boundary = "<<b_header<<std::endl;
		HeaderLineValid(b_header);
		start = end + 2;
		end = boundary_headers.find("\r\n", start);
	}
}

// This function extracts boundary that aim to split blocks in a multipart/form-data request
std::string	Request::extractBoundary(std::string & contentTypeValue)
{
	std::string::size_type	boundary_start = contentTypeValue.find("boundary=");
	if (boundary_start != std::string::npos)
		return "--" + contentTypeValue.substr(boundary_start + 9) ;
	else
		return "";
}

/***********************************************************************************************************
 *                                                                                                         *
 *                                                PROCESS CGI                                              *
 *                                                                                                         *
 ***********************************************************************************************************/

// This function set Env for CGI script
void	Request::setEnv(char **env)
{
	m_pipe.request_method = "REQUEST_METHOD=" + m_method;
	env[0] = &m_pipe.request_method[0];
	m_pipe.script_name = "SCRIPT_NAME=";// + m_pipe.script_name;
	m_pipe.path_info = "PATH_INFO=";// + m_pipe.path_info;
	std::size_t script_end = m_uri.find(".py");
	if (script_end != std::string::npos)// SPLIT script_name and path_info
	{
		std::size_t lastSlash = m_uri.find_last_of("/\\", script_end);
		m_pipe.script_name.insert(m_pipe.script_name.end(), m_uri.begin() + lastSlash + 1, m_uri.begin() + script_end + 3);
		m_pipe.path_info.insert(m_pipe.path_info.end(), m_uri.begin() + script_end + 3, m_uri.end());
		PRINT_GREEN("TEST script name et path info : ")<<m_pipe.script_name<<"  "<<m_pipe.path_info<<std::endl;
		m_uri.erase(script_end + 3);
		PRINT_GREEN("Test de m_uri ")<<m_uri<<std::endl;
	}
	env[1] = &m_pipe.script_name[0];
	env[2] = &m_pipe.path_info[0];
	m_pipe.query_string = "QUERY_STRING=";
	if (!m_query.empty())
		m_pipe.query_string += m_query;
	env[3] = &m_pipe.query_string[0];
	mapString::iterator contentType = m_headers.find("Content-Type:");
	if (contentType != m_headers.end())
		m_pipe.CType = "CONTENT_TYPE=" + contentType->second;
	env[4] = &m_pipe.CType[0];
	mapString::iterator contentLength = m_headers.find("Content-Length:");
	if (contentLength != m_headers.end())
		m_pipe.CLength = "CONTENT_LENGTH=" + contentLength->second;
	// std::cout<<"TEST : "<<CLength<<std::endl;
	env[5] = &m_pipe.CLength[0];
	m_pipe.server = "SERVER_PROTOCOL=HTTP/1.1";
	env[6] = &m_pipe.server[0];
	env[7] = NULL;
}

// This function set arg for execve
void	Request::setArg(char **arg, std::string & interpreter)
{
	std::size_t equal = m_pipe.script_name.find("=");
	m_pipe.script_name.erase(0, equal + 1);
	m_pipe.bin = interpreter;// cgInterpreter
	// PRINT_GREEN("TEST de arg script et interpreter = ")<<m_pipe.script_name<<"  "<<m_pipe.bin<<std::endl;
	arg[0] = &m_pipe.script_name[0];
	// arg[0] = &m_uri[0];
	arg[1] = &m_pipe.bin[0];
	arg[2] = NULL;
}

bool	Request::setPipe()
{
	// PRINT_GREEN("pipe creation")<<std::endl;
	if (pipe(m_pipe.pipe_stdout) == -1)
	{
		// PRINT_RED("Error pipe")<<std::endl;
		m_error_code = 500;
		// PRINT_RED("Mise a false de m_cgi")<<std::endl;
		m_cgi = false; // Pour generer la reponse
		return false;
	}
	if (m_method == "POST")
	{
		if (pipe(m_pipe.pipe_stdin) == -1 )
		{
			// PRINT_RED("Error pipe")<<std::endl;
			m_pipe.clear();
			m_error_code = 500;
			// PRINT_RED("Mise a false de m_cgi")<<std::endl;
			m_cgi = false; // Pour generer la reponse
			return false;
		}
	}
	// PRINT_GREEN("Pipe non bloquant")<<std::endl;
	// MISE EN MODE NON BLOQUANT DES PIPES
	if (fcntl(m_pipe.pipe_stdout[0], F_SETFL, O_NONBLOCK) < 0 || fcntl(m_pipe.pipe_stdout[1], F_SETFL, O_NONBLOCK) < 0)
	{
		// PRINT_GREEN("Error fcntl pipe")<<std::endl;
		m_pipe.clear();
		m_error_code = 500;
		m_cgi = false; // Pour generer la reponse
		return false;
	}
	if (m_method == "POST")
	{
		if (fcntl(m_pipe.pipe_stdin[0], F_SETFL, O_NONBLOCK) < 0 || fcntl(m_pipe.pipe_stdin[1], F_SETFL, O_NONBLOCK) < 0)
		{
			// PRINT_GREEN("Error fcntl pipe")<<std::endl;
			m_pipe.clear();
			m_error_code = 500;
			m_cgi = false; // Pour generer la reponse
			return false;
		}
	}
	return true;
}

// This function closes unused pipe in child process, redirect child stdin and stdout in
// pipe created for this purpose, and execute CGI with env and arg given as argument
void	Request::cgiChildProcess(char **env, char **arg)
{
	if (close(m_pipe.pipe_stdout[0]) == -1)//Pipe de lecture de stdout
	{
		PRINT_RED("Close error in child process")<<std::endl;
		signalHandler(SIGINT);
	}
	if (m_method == "POST")
	{
		if (close(m_pipe.pipe_stdin[1]) == -1)//Pipe d'ecriture stdin
		{
			PRINT_RED("Close error in child process")<<std::endl;
			signalHandler(SIGINT);
		}
	}
	// CHDIR
	std::size_t script_end = m_uri.find(".py");
	m_pipe.working_dir = m_uri;
	std::size_t found = m_pipe.working_dir.find_last_of("/\\", script_end);
	m_pipe.working_dir.erase(found + 1, m_pipe.working_dir.size());
	// PRINT_RED("Test du working_dir = ")<<m_pipe.working_dir<<std::endl;
	if (chdir(m_pipe.working_dir.c_str()) != 0)
	{
		PRINT_RED("Chdir Error in child process")<<std::endl;
		signalHandler(SIGINT);
	}
	// PRINT_RED("TEST1 child process")<<std::endl;
	// REDIRECTION
	if (dup2(m_pipe.pipe_stdout[1], STDOUT_FILENO) == -1)
	{
		PRINT_RED("DUP2 Error in child process")<<std::endl;
		signalHandler(SIGINT);
	}
	if (close(m_pipe.pipe_stdout[1]) == -1)//Pipe de lecture de stdout
	{
		PRINT_RED("Close error in child process")<<std::endl;
		// signalHandler(SIGINT);
	}
	// PRINT_RED("TEST2")<<std::endl;
	if (m_method == "POST")
	{
		if (dup2(m_pipe.pipe_stdin[0], STDIN_FILENO)== -1)
		{
			PRINT_RED("DUP2 Error in child process")<<std::endl;
			signalHandler(SIGINT);
		}
		if (close(m_pipe.pipe_stdin[0]) == -1)//Pipe de lecture de stdout
		{
			PRINT_RED("Close error in child process")<<std::endl;
			// signalHandler(SIGINT);
		}
	}
	// PRINT_RED("TEST3")<<std::endl;
	// EXECVE
	if (execve(arg[0], arg, env) == -1) // Gestion plus precise de code de retour ?? (ex permission denied 403 ??)
	{
		PRINT_RED("Execve failed")<<std::endl;
		std::cerr<<"arg = "<<arg[0]<<" "<<arg[1]<<std::endl;
		// perror("execve");
		signalHandler(SIGINT);
	}
}

// This function closes unused pipe in the parent ans stock pid to kill child process if necessary
void	Request::cgiParentProcess(pid_t pid)
{
	m_pipe.m_pid = pid;
	// PRINT_RED("PID parent = ")<<pid<<std::endl;
	// PRINT_GREEN("TEST PARENT PROCESS")<<std::endl;
	if (close(m_pipe.pipe_stdout[1]) == -1)//Pipe d'ecriture stdout
	{
		PRINT_RED("Close error in parent process")<<std::endl;
		kill(m_pipe.m_pid, SIGINT);
		// attendre la fin du child
		// puis fermer les pipes du parent
		m_error_code = 500;
		m_pipe.clear();
		m_cgi = false; // Pour generer la reponse
		return ;
	}
	m_pipe.pipe_stdout[1] = 0;
	if (m_method == "POST")
	{
		if (close(m_pipe.pipe_stdin[0]) == -1)//Pipe de lecture stdin
		{
			PRINT_RED("Close error in parent process")<<std::endl;
			kill(m_pipe.m_pid, SIGINT);
			// attendre la fin du child
			// puis fermer les pipes du parent
			m_pipe.pipe_stdout[1] = 0;
			m_pipe.clear();
			m_error_code = 500;
			m_cgi = false; // Pour generer la reponse
		}
		m_pipe.pipe_stdin[0] = 0;
	}
	// std::cout<<"pipe_stdin[] == "<<m_pipe.pipe_stdin[1]<<" pipe_stdout[0] == "<<m_pipe.pipe_stdout[0]<<std::endl;
}

// This function handle a cgi request
void	Request::processCGI(ServerConfig & config, int loc_index)
{
	// PRINT_GREEN("ProcessCGI called")<<std::endl;
	if (m_method == "DELETE")
	{
		m_error_code = 403;
		// m_cgi = false; // Pour generer la reponse
		return ;
	}
	m_cgi = true; // Pour faire cette logique une fois seulement
	// ChHECK RESSOURCE VALIDITY
	// std::cout<<"TEST dans process CGI de uri : "<<m_uri<<std::endl;
	// PRINT_GREEN("set Env")<<std::endl;
	char *env[8];
	setEnv(env);
	if (!checkRessourceAccessibilty(m_uri))
		return ;
	if (m_uri.size() < 3 || m_uri.find(".py") == std::string::npos)
	{
		PRINT_RED("Ressource doesn't have correct format [.py]")<<std::endl;
		m_error_code = 400;
		return ;
	}
	// SET ENV / SET ARG
	char *arg[3];
	// PRINT_GREEN("set Arg")<<std::endl;
	setArg(arg, config._locations[loc_index]._cgiInterpreter);
	// PIPE CREATION
	if (!setPipe())
		return ;
	// FORK
	// PRINT_GREEN("Fork")<<std::endl;
	pid_t pid = fork();
	if (pid == 0)
	{
		// PRINT_RED("PID child = ")<<pid<<std::endl;
		cgiChildProcess(env, arg);
	}
	else if ( pid > 0)//Parent process
		cgiParentProcess(pid);
	else // Fork error
	{
		PRINT_RED("Fork error : ")<<strerror(errno)<<std::endl;
		m_pipe.clear();
		m_error_code = 500;
		m_cgi = false;
	}
}

/***********************************************************************************************************
 *                                                                                                         *
 *                                                MAINTENANCE                                              *
 *                                                                                                         *
 ***********************************************************************************************************/

// This function specifies if a Request has to be interpreted as keep-alive connection or not
// In fact only when header Connection specified close, it is not interpreted as keep-alive connection
bool	Request::isKeepAlive()
{
	mapString::iterator it = m_headers.find("Connection:");
	if (it != m_headers.end())
	{
		// PRINT_GREEN("Connection: Header trouve")<<std::endl;
		if (m_headers["Connection:"] == "close")
			return false;
		else
			return true;
	}
	return true;
}

void	Request::clear()
{
	// PRINT_RED("Mise a false de m_cgi [request.clear] : ")<<m_uri<<std::endl;
	m_read.clear();
	m_chunked_body.clear();
	m_ready = false;
	m_chunked = false;
	m_end_of_chunked = false;
	m_method.clear();
	m_uri.clear();
	m_uriIsADirectory = false;
	m_query.clear();
	m_protocol.clear();
	m_body_pos = 0;
	m_error_code = 0;
	m_response_code = 0;
	m_headers.clear();
	m_cgi = false;
	m_filename.clear();
	m_pipe.clear(); // Bool remis a false [Pipe a checker]
	m_root.clear();
	m_redirection = false;
	// m_pipe => normalement rien a faire puisque tout sera close et a 0a faire en fin de reponse HTTP
}

//			-Penser a l'affichage final

//			-Possible derniers petits cas de Broken pipe => Solution prioritaire faire un waitpid apres chaque kill
//			-Un cas de broken pipe que je ne retrouve plus. C'est dans un enchainement de requete CGI.
//			-Cas d'un script cgi boucle infini + une autre requete = Brokenpipe car dans la fonction handlingKeepAlive
//			 pour une raison inconnue m_cgi est a false et donc on ne kill pas le child process qui est encore dans une boucle infinie
//			 Edit : Probleme de constructeur de copy et d'assignation dans mes classe qui sont utilise dans la logique de vecteur donc lors de l'acceptation
//					d'une nouvelle connexion la connexion CGI etait comme reset puisque mal copie donc considere comme non CGI donc pas de kill et de close des pipe
//					Aussi, se posait leprobleme de l'appel a close dans le destructeur de request et ou PipeHandler
//					A METTRE DANS LE ACCEPT


//	-Check correction

//	Probleme CGI :
//		-"Exception ignored in: <_io.TextIOWrapper name='<stdout>' mode='w' encoding='utf-8'>
// 			BlockingIOError: [Errno 11] write could not complete without blocking"
//		lorsque j'envoie de la donnee de maniere consequante au script CGI.
//		P
//		Ce probleme n'est plus present lorsque je mets les pipes en mode Bloquant
//		- Pipe ouvert et "valgrind: script.py: command not found" lorsque je mets tout les flags valgrind
//		  Lorsque j'eneleve track-children pas de pipe ouvert et le premier probleme intervient


//Solution possible
//	Faire le chdir dans le parent et
