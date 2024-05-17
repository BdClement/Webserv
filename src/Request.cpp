/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/13 12:29:28 by clbernar          #+#    #+#             */
/*   Updated: 2024/05/17 14:12:24 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request() : m_requestIsComplete(false), m_body_pos(0), m_error_code(0), m_headers()
{
	// std::cout<<"Request constructor called"<<std::endl;
}

Request::Request(Request const& asign)
{
	m_read = asign.m_read;
	m_requestIsComplete = asign.m_requestIsComplete;
	// std::cout<<"Request copy constructor called"<<std::endl;
}

Request::~Request()
{
	// std::cout<<"Request destructor called"<<std::endl;
}

Request& Request::operator=(Request const & equal)
{
	if (this != &equal)
	{
		m_read = equal.m_read;
		m_requestIsComplete = equal.m_requestIsComplete;
	}
	return *this;
}

void	Request::parseRequest()
{
	unsigned char	endOfHeaders[] = {'\r', '\n', '\r', '\n'};
	std::vector<unsigned char>::iterator	it;
	it = std::search(this->m_read.begin(), this->m_read.end(), endOfHeaders, endOfHeaders + 4);
	if (it != this->m_read.end())
	{
		m_body_pos = it - m_read.begin();
		std::cout<<"START PARSING"<<std::endl;
		size_t startHeaders = parseRequestLine();
		parseHeaders(startHeaders);
		// Ajout d'une logique de BODY et d'une suite de body ??
		m_requestIsComplete = true;// On ne gere pas pour l'instant de requete contenant un body
	}
	else
		std::cout<<"La requete n'as pas encore ete lue jusqu'a la fin des Headers"<<std::endl;
	// Ajouter Logique de timeout de Connection pour different cas
}

// This function parse the RequestLine (the first one)
// A good RequestLine request 3 elements [Method URI Protocol]
size_t	Request::parseRequestLine()
{
	std::string	requestLine(m_read.begin(), m_read.end());
	size_t	pos = requestLine.find("\r\n");
	// std::cout<<"Affichage de pos = ["<<m_read[pos + 2]<<"]"<<std::endl;
	if (pos != std::string::npos)
	{
		// Checker la methode : 2 cas Not Implemented et Not Allowed
		// Checker le protocole
		std::string	line = requestLine.substr(0, pos);
		size_t	space1 = line.find(' ');
		size_t	space2 = line.find(' ', space1 + 1);
		// CHECK MORE THAN 3 ELEMENTS
		size_t	space3 = line.find(' ', space2 + 1);
		if (space3 != std::string::npos)
		{
			m_error_code = 400;

		}// CHECK LESS THAN 3 ELEMENTS
		else if (space1 != std::string::npos && space2 != std::string::npos)
		{
			m_method = line.substr(0, space1);
			std::string uri = line.substr(space1 + 1, space2 - space1 - 1);
			m_protocol = line.substr(space2 + 1);
			if (!MethodAllowed(m_method))
			{
				PRINT_RED("Method Not Allowed")<<std::endl;
				m_error_code = 405;
			}
			else if (!UriValid(uri))
				return 0;
			else if (!ProtocolValid(m_protocol))
			{
				PRINT_RED("Protocol Not Supported")<<std::endl;
				m_error_code = 505;
			}
		}
		else
			m_error_code = 400;
	}
	else
		m_error_code = 400;
	PRINT_GREEN("Resultat de fin de parsing de Request Line :")<<std::endl;
	std::cout<<"method = "<<m_method<<std::endl;
	std::cout<<"Uri = "<<m_uri<<std::endl;
	std::cout<<"protocol = "<<m_protocol<<std::endl;
	// std::string test("/MyPage.html?param1=salut&param2=cava");
	// std::cout<<"Test de URI avec query string :"<<UriValid(test)<<std::endl;
	// std::cout<<"Uri = "<<m_uri<<std::endl;
	// std::cout<<"Query = "<<m_query<<std::endl;
	return pos + 2;// Pour conserver la requete originale mais une fois que tout est bon modifier en supprimant ce qui a ete parser pour garder uniquement le body
}

bool	Request::MethodAllowed(std::string const & method) const
{
	return method == "GET" || method == "POST" || method == "DELETE";
}

// conserver 1.0 ??
bool	Request::ProtocolValid(std::string const & protocol) const
{
	return protocol == "HTTP/1.1" || protocol == "HTTP/1.0";
}

// Certains caracteres dit reserves (cf RFC) doivent etre interpretes ou non dans certains cas
// Pour un serveur simple comme webserv, j'autorise l'ensemble de ces caracteres que je n'interpreterais pas
// L'utilisation de ces caracteres devra donc forcement correspondre a une ressource existante pour ne pas
// engendrer d'erreur.
// RFC 3986 concernant les URI
// Mettre en place une logique qui cherche si la ressource se trouve dans un chemin autorise par le serveur ??
bool	Request::UriValid(std::string & uri)
{
	// URI SIZE
	if (uri.size() > URI_SIZE_MAX)
	{
		m_error_code = 414;
		PRINT_RED("URI is too large")<<std::endl;
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
	if (m_error_code != 0)// Check Error Parsing RequestLine
		return;
	std::string	Headers(m_read.begin() + startHeaders, m_read.end());
	// CHECK DE LA SIZE DE L'ENSEMBLE DES HEADERS
	if (Headers.size() == 0)
	{
		m_error_code = 400;
		PRINT_RED("WARNING : No Headers, Suspicious activity")<<std::endl;
		return ;
	}
	else if (Headers.size() > HEADERTOTAL_MAX_SIZE)
	{
		m_error_code = 431;
		PRINT_RED("Request Fields Too Large")<<std::endl;
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
		// sleep(1);
		initial_pos = pos + 2;
		// std::cout<<"TEST"<<std::endl;
	}
	// Inclusion dans la map
	// PRINT_GREEN("AFFICHAGE DE MES HEADERS :\n")<<Headers<<std::endl;
	// std::cout<<"AFFICHAGE DE MA POSITION de fin de headers : "<<m_read[m_body_pos - 1]<<std::endl;
	// m_body_pos est ok = Si il y a un Body, il start a m_body_pos + 4 [A CHECKER]
	PRINT_GREEN("Resultat de fin de parsing des Headers :")<<std::endl;
	mapString::iterator it;
	for (it = m_headers.begin(); it != m_headers.end(); ++it)
	{
		std::cout<<it->first<<" "<<it->second<<std::endl;
	}
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
		PRINT_RED("Request Fields Too Large")<<std::endl;
		return false;
	}
	// SPLIT DE HEADERLINE POUR CHECKER LE FORMAT [key: value]
	size_t	space = header.find(' ');
	if (space == std::string::npos)// NO SPACE ON HEADERLINE
	{
		std::cout<<"No space in header line => Wrong syntax"<<std::endl;
		m_error_code = 400;
		return false;
	}
	else//CHECK DE LA SYNTAXE [key: value]
	{
		std::string key = header.substr(0, space);
		if (key[key.size() - 1] != ':')// PAS DE ":" A A FIN DE KEY dans [key: value]
		{
			m_error_code = 400;
			PRINT_RED("PAS LA BONNE SYNTAXE")<<" Last element de key: == "<<key[key.size() - 1]<<std::endl;
			return false;
		}
		// else
		// 	PRINT_GREEN("HEADER A LA BONNE SYNTAXE")<<std::endl;
		std::string value = header.substr(space + 1);
		// std::cout<<"Format [key:][value] == ["<<key<<"]["<<value<<"]"<<std::endl;
		// STOCKAGE DU HEADER
		m_headers[key] = value;
	}
	return true;
}

// A faire :
//			-Test les headers avec curl
//			-Implmenter une logique de reponse pour les erreurs
//			-Implmenter une logique de reponse basique pourles requetes basiques
