/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/13 12:29:28 by clbernar          #+#    #+#             */
/*   Updated: 2024/06/06 19:23:22 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request() : m_ready(false), m_chunked(false), m_end_of_chunked(false), m_uriIsADirectory(false), m_body_pos(0), m_error_code(0), m_response_code(0), m_headers()//, m_left_to_read(0), m_already_stocked(0)
{
	// std::cout<<"Request constructor called"<<std::endl;
}

Request::Request(Request const& asign)
{
	m_read = asign.m_read;
	m_ready = asign.m_ready;
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
		m_ready = equal.m_ready;
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
			m_ready = true;
		// else
			//Logique de Parsing de ce qui vient d'etre lu
		else if (m_end_of_chunked)
		{
			// PRINT_GREEN("Ici devra etre mis en place un parsing special de Body Pour cleaner les chunked")<<std::endl;
			m_ready = true;
		}
	}
	// else
		// std::cout<<"La requete n'as pas encore ete lue jusqu'a la fin des Headers"<<std::endl;
}

// Checker si le client peut envoyer une nouvelle requete alors qu'il na pas eud e reponse a la precedente DISCORD
// Attention cas ou le chunked de fin est recu et donc on repond a la requete mais que de la donnee est encore envoye sur la socket
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

// curl -X POST -d "" http://example.com/path
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
			m_error_code = 400;
	}
	else
		m_error_code = 400;
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
	errno = 0;
	try
	{
		content_length = strtoll(value.c_str(), &end, 10);
		if (end == value.c_str() || *end != '\0')
			throw std::invalid_argument("Invalid Content-Length value");
		if (content_length < 0 || content_length > INT_MAX)
			throw std::out_of_range("Content-Length value out of range");
		if (content_length == 0)// On ne permet pas de content length 0 ?? Ou sinon rajouter une condition dans generateResponse Quand aucun error_code ni response code
		{//							n'est present on met automatiquement 400 ??
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

// This function check if the server is waiting for a Body and it has been completely read
// To process Request
void	Request::checkBody()
{
	// std::cout<<"Body position element == "<<m_read[m_body_pos]<<std::endl;
	// std::cout<<" Body pos == "<<m_body_pos<<std::endl;
	if (m_method == "POST" && !m_chunked)
	{
		if (m_read.size() > m_body_pos + 4)// Place m_body_pos au debut du Body s'il y en a un //  A RETIRER
			m_body_pos += 4; // A tester cas de POST SANS BODY
		mapString::iterator it = m_headers.find("Content-Length:");
		if (it != m_headers.end())// Si Content-Length est present pour POST
		{
			long long content_length = 0;
			// std::cout<<"Body = "<<m_read.size() - m_body_pos<<" pour un body de taille "<<it->second<<std::endl;
			if (!checkContentLengthValue(it->second, content_length)) //Si la valeur de Content-Length est invalide
				return ;
			// std::cout<<"readSize | BodyPos | contentLength == "<<m_read.size()<<"  "<<m_body_pos<<"  "<<content_length<<std::endl;
			if (m_read.size() - m_body_pos < (unsigned long)content_length)// Si Content-Length n'a pas ete lu en entier
				m_error_code = 400;
			else if (m_read.size() - m_body_pos > (unsigned long)content_length)// Si plus que Content-Length a ete lu
				m_error_code = 413;
		}
		else// Si Content-Length n'est pas present pour POST
		{
			// PRINT_RED("Content-Length Pas prsent dans la requete");
			m_error_code = 411;// Length Required
		}
	}
}

/***********************************************************************************************************
 *                                                                                                         *
 *                                              PROCESS REQUEST                                            *
 *                                                                                                         *
 ***********************************************************************************************************/

void	Request::processRequest(std::vector<Config> & m_config, Response & response)
{
	// FIND VIRTUAL SERVER
	// FIND LOCATION??
	// Check CGI
	if (m_method == "GET")
		processGet(m_config[0], response);
	else if (m_method == "POST")
		processPost(m_config[0], response);
	else if (m_method == "DELETE")
		processDelete(m_config[0], response);
}

// Attention Header Accept contenu ce qui est envoye ou renvoyer ??
// This function process a GET Request. If an error occured,it specifies it and stops
// If not,the ressource asked is placed in the Response object
void	Request::processGet(Config & config, Response & response)
{
	// DIFFERENTS CHECK DU A LA CONFIG OU AU HEADER DE LA REQUETE ?
	// Recherche du bloc Location ?
	// Check CGI ou pas
	// en vrai il faudrait trouver la base equivalent a "/test" ici dans l'objet config
	// DEFINITION DE RESSOURCE GRACE A LA CONFIG
	(void)config;
	std::string ressource = "test" + m_uri; // Attention au cas ou ressource est un Directory
	if (!checkRessourceAccessibilty(ressource))
		return ;
	// PRINT_RED("Probleme avec la resource demande : ")<<ressource<<" "<<strerror(errno)<<std::endl;// A suuprimer
	std::ifstream file(ressource.c_str(), std::ios::binary);
	if (!file)
	{
		PRINT_RED("Impossible to open ressource : Strange check processGet function")<<std::endl;
		m_error_code = 500;// Checker d'autres cas possibles ?
		return ;
	}
	// else
	// 	PRINT_GREEN("Ressource trouvee : ")<<ressource<<std::endl;
	response.m_response = std::vector<unsigned char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	response.m_body_size = response.m_response.size();
	m_response_code = 200;// A voir les cas de variantes ??
	file.close();
	response.m_response.push_back('\r');
	response.m_response.push_back('\n');
}

// This function checks the ressource asked in a request
// If it exists, is not a directory and has permission access,it returns true
bool	Request::checkRessourceAccessibilty(std::string const & ressource)
{
	struct stat s;
	// EXISTENCE
	if (stat(ressource.c_str(), &s) != 0)// La ressource n'existe pas
	{
		PRINT_RED("Ressource doesn't exist")<<std::endl;
		if (m_method != "POST")
			m_error_code = 404;
		return false;
	}
	// DIRECTORY
	if (s.st_mode & S_IFDIR)// A gerer avec la Config (Pour GET notamment)
	{
		PRINT_RED("Ressource is a directory")<<std::endl;
		// Pour DELTETE en cas de directory , check si la methode est GET pour agir differemment (+POST par la suite)
		m_error_code = 405;// Pas autorise par DELETE. Check potentiellement 403 ?
		// IF GET et PAS d'index specifie Et pas autoindex active => 403 Forbidden
		// La difference est que GET est autorise sur un directory mais depend de la config
		// Alors que
		return false;
	}
	// PERMISSION
	if (m_method == "GET")
	{
		if (access(ressource.c_str(), R_OK) == -1)//Checker ordre : Check permission avant directory ?
		{
			PRINT_RED("Ressource doesn't have permission access")<<std::endl;
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
void	Request::processDelete(Config & config, Response & response)
{
	(void)config;
	// DIFFERENTS CHECK DU A LA CONFIG OU AU HEADER DE LA REQUETE ?
	// Recherche du bloc Location ?
	// Check CGI ou pas => Error 405 ?
	// Initialisation de ressource grace a la config
	std::string ressource = "test" + m_uri;
	if (!checkRessourceAccessibilty(ressource))
		return ;
	else// FILE
	{
		std::string body;
		PRINT_GREEN("Ressource is a file")<<std::endl;
		if (std::remove(ressource.c_str()) == 0)// DELETED
		{
			body = "\r\nRessource has been deleted\r\n";
			response.m_response.insert(response.m_response.end(), body.begin(), body.end());
			m_response_code = 200;
		}
		else// NOT DELETED
		{
			PRINT_RED("File could't be deleted")<<std::endl;
			// body = "Ressource couldn't be deleted\r\n";
			// response.m_response.insert(response.m_response.end(), body.begin(), body.end());
			m_error_code = 500;// ?? OU 403 Pa de permission si le parent n'a pas les permissions
		}
		response.m_content_type = "Content-Type: text/plain\r\n";
	}
}

// En cas de succes Message sous forme de text en tant que Body
// "Data received and processed successfully."|| "File uploaded successfully."
void	Request::processPost(Config & config, Response & response)
{
	(void)config;
	(void)response;
	// DIFFERENTS CHECK DU A LA CONFIG OU AU HEADER DE LA REQUETE ?
	// Recherche du bloc Location ?
	// Check CGI ou pas
	// PRINT_GREEN("BODY RECEIVED SIZE : ")<<std::distance(m_read.begin() + m_body_pos, m_read.end())<<std::endl;
	// std::cout<<"\nRequest received :\n["<<std::endl;
	// for (std::vector<unsigned char>::iterator it = m_read.begin(); it != m_read.end(); ++it)// AFFICHAGE DE TEST
	// 		std::cout<<*it;
	// std::cout<<" ]"<<std::endl;
	// std::string ressource = "test" + m_uri;
	// PRINT_GREEN("processPost Called")<<std::endl;
	std::vector<unsigned char>	request_body(m_read.begin() + m_body_pos, m_read.end());
	mapString::iterator it = m_headers.find("Content-Type:");
	if (it != m_headers.end() && (it->second.find("multipart/form-data") != std::string::npos))
		processPostMultipart(extractBoundary(it->second), request_body);
	else if (checkFileUpload()) // Televersement Simple A CHECKER !!!!!
		uploadFile(request_body);
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
	// std::vector<unsigned char>	body(m_read.begin() + m_body_pos, m_read.end());
	// PRINT_RED("Le body extrait de m_read est de taille : ")<<body.size()<<std::endl;
	FILE* file = fopen("test/DataBaseWebserv.txt", "ab");// en mode binaire append
	if (!file)
	{
		PRINT_RED("Error : Imposible to open Webserv Data Base.")<<std::endl;
		m_error_code = 500;
		return ;
	}
	size_t written = fwrite(&body[0], 1, body.size(), file);
	if (written != body.size())
	{
		PRINT_RED("Error writting Data Base")<<std::endl;
		m_error_code = 500;// ???
	}
	else
	{
		m_response_code = 201;// A checker
		fputc('\n', file);
		fputc('\n', file);
	}
	fclose(file);
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
void	Request::uploadFile(std::vector<unsigned char> &boundary_body)
{
	if (boundary_body.size() == 0 || m_filename.size() == 0)
		return ;
	std::string location = "test/upload/" + m_filename;
	if	(checkRessourceAccessibilty(location))
	{
		m_error_code = 409;//Confilct exsite deja !
		return ;
	}
	else
	{
		FILE* file = fopen(location.c_str(), "wb");// en mode write binaire
		if (!file)
		{
			PRINT_RED("Error : Imposible to open file.")<<std::endl;
			m_error_code = 500;
			return ;
		}
		size_t written = fwrite(&boundary_body[0], 1, boundary_body.size(), file);
		if (written != boundary_body.size())
		{
			PRINT_RED("Error writting file.")<<std::endl;
			m_error_code = 500;
		}
		else
			m_response_code = 201;
		fclose(file);
	}
}

// This function is the main loop for processing each block of a multipart/form-data request
// It splits each block and sends it to processPostBoundaryBlock()
void	Request::processPostMultipart(std::string boundary, std::vector<unsigned char> & body)
{
	if (boundary.size() == 0)
	{
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
		processPostBoundaryBlock(boundary_block);
		// EFFACE LE BLOCK DEJA TRAITE
		body.erase(body.begin(), next);// On efface le block qui a ete traite
		// if (it == std::search(body.begin(), body.end(), end_boundary.begin(), end_boundary.end()))// a remettre dans le while ??????!!!
		// 	break ;
	}
	if (m_error_code != 0 && m_response_code != 0)// Cas multiple Voir le comportement adequat 207 ??
	{
		m_response_code = 0;
		m_error_code = 400;
	}
	// Multipart impossible avec CGI ?
}

// Code de reponse :
//		Si tout le monde reussit : 200 ou 201
//		Tout le monde ne reussit pas mais certain reussisent : 400 prioritaire
//		Tout le mond echoue : 400 Prioritaire si il yen a un sinon
// This function recveive a multipart/form-data block, strores new headers from it thanks to updateHeaders()
// Depending on new headers result, it process the request by storing data or upload file
void	Request::processPostBoundaryBlock(std::vector<unsigned char> &boundary_block)
{
	// PRINT
	// PRINT_GREEN("Traitement du block boundary")<<std::endl;
	// for (std::vector<unsigned char>::iterator it = boundary_block.begin(); it != boundary_block.end(); ++it)
	// 	std::cout<<*it;
	// SPLIT HEADERS / BODY
	unsigned char	endOfHeaders[] = {'\r', '\n', '\r', '\n'};
	std::vector<unsigned char>::iterator	it;
	it = std::search(boundary_block.begin(), boundary_block.end(), endOfHeaders, endOfHeaders + 4);
	if (it == boundary_block.end())
	{
		// PRINT_RED("Pas de fin de Headers trouve dans le block boundary")<<std::endl;
		m_error_code = 400;
		return ;
	}
	// STRING CONTENANT L'ENSEMBLE DES HEADERS
	// UPDATE m_headers
	std::string boundary_headers(boundary_block.begin(), it + 4);
	// std::cout<<"Affichage des boundary headers ::   "<<boundary_headers<<std::endl;
	if (it + 4 == boundary_block.end())// S'IL N'Y A PAS DE BODY
	{
		m_error_code = 400;
		return ;
	}
	std::vector<unsigned char> boundary_body(it + 4, boundary_block.end());
	// std::cout<<"Affichage du boundary body : "<<std::endl;
	// for (std::vector<unsigned char>::iterator itest = boundary_body.begin(); itest != boundary_body.end(); ++itest)
	// 	std::cout<<*itest;
	updateHeaders(boundary_headers);
	if (checkFileUpload())
	{
		// PRINT_RED("UPLOAD MULTIPART")<<std::endl;
		uploadFile(boundary_body);//Reset multi part a inserer ici je pense
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
	// m_left_to_read = 0;
	// m_already_stocked = 0;
}

// A faire :
//			-Merge /
//			-Implementations des fonctionnalites propre a la config
//			-CGI

//			-Penser a l'affichage final
//			-Mettre une limite de fichier upload adequate ?
//			-Gerer les code de reponse pour multipart ! (On verra lors du testeur) + bloc vide multipart (400 ? pour etre coherent avec le reste)
