/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 16:05:07 by clbernar          #+#    #+#             */
/*   Updated: 2024/07/06 15:27:20 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "Request.hpp"

Response::Response() : m_body_size(0)
{
	// std::cout<<"Response constructor called"<<std::endl;
	set_error_file();
	set_code_meaning();
}

Response::Response(Response const& asign)
{
	// std::cout<<"Response copy constructor called"<<std::endl;
	(void)asign;
}

Response::~Response()
{
	// std::cout<<"Response destructor called"<<std::endl;
}

Response& Response::operator=(Response const & equal)
{
	if (this != &equal)
	{
		(void)equal;
	}
	return *this;
}

void	Response::set_error_file()
{
	m_error_file[400] = "racine/error/400BadRequest.html";
	m_error_file[403] = "racine/error/403Forbidden.html";
	m_error_file[404] = "racine/error/404NotFound.html";
	m_error_file[405] = "racine/error/405MethodNotAllowed.html";
	m_error_file[409] = "racine/error/409Conflict.html";
	m_error_file[411] = "racine/error/411LengthRequired.html";
	m_error_file[413] = "racine/error/413PayloadTooLarge.html";
	m_error_file[414] = "racine/error/414UriTooLarge.html";
	m_error_file[415] = "racine/error/UriTooLarge.html";
	m_error_file[431] = "racine/error/RequestHeaderFieldsTooLarge.html";
	m_error_file[500] = "racine/error/500InternalServerError.html";
	m_error_file[501] = "racine/error/501NotImplemented.html";
	m_error_file[503] = "racine/error/503ServiceUnavailable.html";
	m_error_file[505] = "racine/error/505HTTPVersionNotSupported.html";
}

void	Response::set_code_meaning()
{
	m_code_meaning[200] = "OK";// Requete reussi
	m_code_meaning[201] = "Created";//Ressource cree avec succes (POST)
	m_code_meaning[204] = "No Content";//Requete reussi mais pas de contenua retourne (Delete ?)
	m_code_meaning[301] = "Moved Permanently";//La ressource demandée a été déplacée de façon permanente à une nouvelle URL
	m_code_meaning[302] = "Found";// La ressource demandée réside temporairement sous une URL différente.
	m_code_meaning[304] = "Not Modified";//304 Not Modified : La ressource n'a pas été modifiée depuis la dernière requête ??
	m_code_meaning[400] = "Bad Request";//La requête est mal formulée
	m_code_meaning[401] = "Unauthorized";// Authentification nécessaire et non fournie ??
	m_code_meaning[403] = "Forbidden";//Serveur a compris la requête mais refuse de l'autoriser
	m_code_meaning[404] = "Not Found";//La ressource demandée n'a pas été trouvée
	m_code_meaning[409] = "Conflict";
	m_code_meaning[411] = "Length Required";
	m_code_meaning[413] = "Payload Too Large";
	m_code_meaning[414] = "Uri Too Large";
	m_code_meaning[405] = "Method Not Allowed";//La méthode demandée n'est pas autorisée pour la ressource
	m_code_meaning[500] = "Internal Server Error";//Erreur interne du serveur
	m_code_meaning[501] = "Not Implemented";//  La méthode demandée n'est pas supportée par le serveur
	m_code_meaning[503] = "Service Unavailable";//Le serveur est actuellement incapable de traiter la requête.
	m_code_meaning[505] = "HTTP Version Not Supported";// Le protocol n'est pas supporte par le serveur
}

void Response::findErrorPage(ServerConfig & serverBlock, int error_code, std::string & root)
{
	std::map<int, std::string>::iterator it = serverBlock._errorPages.find(error_code);
	if (it != serverBlock._errorPages.end())
		m_error_page = root + it->second;
	// std::cout<<"TEST de m_error_page dans findErrorPage : "<<m_error_page<<std::endl;
	// PRINT_GREEN("Test de root : ")<<root<<" Et de size de root : "<<root.size()<<std::endl;
}

void	Response::generateRedirectionResponse(Request & request, Location & locationBlock)
{
	request.m_redirection = true;
	int response = std::atoi(locationBlock._return[0].c_str());
	request.m_response_code = response;
	std::string header = "Location: " + locationBlock._return[1] + "\r\n";
	// generateHeaders(request);
	m_response.insert(m_response.begin(), header.begin(), header.end());
	generateStatusLine(request);
}

void	Response::generateResponse(Request & request, ServerConfig & serverBlock, int loc_index)
{
	if (request.m_error_code == 0 && loc_index != -1 && serverBlock._locations[loc_index]._return.size() != 0)
	{
		generateRedirectionResponse(request, serverBlock._locations[loc_index]);
		return ;
	}
	if (request.m_error_code != 0)
	{
		findErrorPage(serverBlock, request.m_error_code, request.m_root);
		generateErrorBody(request);
	}
	generateHeaders(request);
	generateStatusLine(request);
	request.m_body_pos = 0;
}

// This function generate Response's Status Line
// Format = [ HTTP/1.1 CODE COMMENT ]
void	Response::generateStatusLine(Request & request)
{
	std::string statusLine = "HTTP/1.1 ";
	std::string code_meaning;
	std::stringstream code;
	if (request.m_error_code == 0 && request.m_response_code == 0)
		request.m_error_code = 500;
	if (request.m_error_code != 0)
	{
		code << request.m_error_code;
		mapIntString::iterator it = m_code_meaning.find(request.m_error_code);
		if (it != m_code_meaning.end())
			code_meaning = " " + m_code_meaning[request.m_error_code];
	}
	else
	{
		code << request.m_response_code;
		mapIntString::iterator it = m_code_meaning.find(request.m_response_code);
		if (it != m_code_meaning.end())
			code_meaning = " " + m_code_meaning[request.m_response_code];
	}
	code_meaning += "\r\n";
	statusLine += code.str() + code_meaning;
	m_response.insert(m_response.begin(), statusLine.begin(), statusLine.end());
	// PRINT_GREEN("Contenu de la reponse a generateStatusLine = ")<<std::endl;
	// for (std::vector<unsigned char>::iterator it = m_response.begin(); it != m_response.end(); ++it)
	// 	std::cout<<*it;
	// std::cout<<std::endl;
	// PRINT_GREEN("Taille de la reponse a generateStatusLine ")<<m_response.size()<<std::endl;
}

// This function generate appropriate headers depending on the Request state
void	Response::generateHeaders(Request & request)
{
	(void)request;
	std::string crlf = "\r\n";
	// if (m_response.size() != 0)
	m_response.insert(m_response.begin(), crlf.begin(), crlf.end());
	if (m_response.size() != 2)
	{
		std::stringstream value;
		size_t size = m_response.size() - 2;
		value << size;
		std::string contentLength = "Content-Length: " + value.str() + "\r\n";
		m_response.insert(m_response.begin(), contentLength.begin(), contentLength.end());
		//Content-Type
		if (m_content_type.size() == 0)
			setContentType(request.m_uri);
		m_response.insert(m_response.begin(), m_content_type.begin(), m_content_type.end());
	}
	// if (m_body_size != 0) // IF BODY
	// {
	// 	// Content-Length
	// 	std::stringstream ss;
	// 	ss << m_body_size;
	// 	std::string contentLength = "Content-Length: " + ss.str() + "\r\n";
	// 	m_response.insert(m_response.begin(), contentLength.begin(), contentLength.end());
	// 	//Content-Type
	// 	if (m_content_type.size() == 0)
	// 		setContentType(request.m_uri);
	// 	m_response.insert(m_response.begin(), m_content_type.begin(), m_content_type.end());
	// }
	// Server [Facultatif]
	std::string server = "Server: HTTP/1.1 webserv\r\n";
	m_response.insert(m_response.begin(), server.begin(), server.end());
	// Connection
	mapString::iterator it = request.m_headers.find("Connection:");
	std::string connection;
	if (it != request.m_headers.end())
	{
		if (request.m_headers["Connection:"] == "close")
			connection = "Connection: close\r\n";
		else
			connection = "Connection: keep-alive\r\n";
	}
	else
	{
		connection = "Connection: keep-alive\r\n";
	}
	m_response.insert(m_response.begin(), connection.begin(), connection.end());
	// PRINT_GREEN("Contenu de la reponse a generateHeaders = ")<<std::endl;
	// for (std::vector<unsigned char>::iterator it = m_response.begin(); it != m_response.end(); ++it)
	// 	std::cout<<*it;
	// std::cout<<std::endl;
	// PRINT_GREEN("Taille de la reponse a generateHeaders ")<<m_response.size()<<std::endl;
}

// This function checks the ressource's extension to set the Content-Type header
// It is called when Content-Type hasn't been set already (Error found or Content predefined)
void	Response::setContentType(std::string & uri)
{
	// std::cout<<"TEST de setContentType uri : "<<uri<<std::endl;
	size_t	point = uri.find_last_of('.');
	if (point != std::string::npos)
	{
		std::string extension = uri.substr(point + 1);
		// std::cout<<"TEST de setContentType : uri / extension : "<<uri<<" "<<extension<<std::endl;
		if (extension == "html")
			m_content_type = "Content-Type: text/html\r\n";
		else if (extension == "jpeg" || extension == "jpg")
			m_content_type = "Content-Type: image/jpeg\r\n";
		else if (extension == "png")
			m_content_type = "Content-Type: image/png\r\n";
		else if (extension == "pdf")
			m_content_type = "Content-Type: application/pdf\r\n";
		else if (extension == "txt" || extension == "text")
			m_content_type = "Content-Type: text/plain\r\n";
		else if (extension == "css")
			m_content_type= "Content-Type: text/css\r\n";
		else if (extension == "js")
			m_content_type = "Content-Type: application/javascript\r\n";

	}
	else
		m_content_type = "Content-Type: application/octet-stream\r\n";
}

// Cas du CGI qui produira potentiellement un Body
// This function is called when an error occured to send appropriate HTML error file
void	Response::generateErrorBody(Request & request)
{
	if (request.m_error_code != 0)// Sinon le body est deja inscrit dans la reponse OU il n'y a pas de body
	{
		std::ifstream file;
		if (!m_error_page.empty())
		{
			// PRINT_GREEN("TEST1")<<std::endl;
			file.open(m_error_page.c_str());
			if (!file)
			{
				// PRINT_GREEN("TEST2")<<std::endl;
				m_error_page.clear();
				generateErrorBody(request);
			}
		}
		else
		{
			// PRINT_GREEN("TEST3")<<std::endl;
			mapIntString::iterator it = m_error_file.find(request.m_error_code);
			if (it == m_error_file.end())// Recherche du fichier d'erreur associe a ce code
			{
				// PRINT_GREEN("TEST4")<<std::endl;
				PRINT_RED("Unknown error code : ")<<request.m_error_code<<std::endl;
				return ;
			}
			file.open(it->second.c_str());// Ouverture du fichier d'erreur trouvee
		}
		if (!file)
		{
			// PRINT_GREEN("TEST5")<<std::endl;
			PRINT_RED("Probleme fichier d'erreur : ")<<strerror(errno)<<std::endl;// errno a supprimer ??!
			return ;
		}
		//Remplissage de la reponse avec le fichier d'erreur
		// PRINT_GREEN("TEST6")<<std::endl;
		m_response = std::vector<unsigned char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		m_body_size = m_response.size() + 2;
		m_response.push_back('\r');
		m_response.push_back('\n');
		m_content_type = "Content-Type: text/html\r\n";
		// PRINT_GREEN("Contenu de la reponse a generateBody = ")<<std::endl;
		// for (std::vector<unsigned char>::iterator it = m_response.begin(); it != m_response.end(); ++it)
		// 	std::cout<<*it;
		// std::cout<<std::endl;
		// PRINT_GREEN("Taille de la reponse a generateBody ")<<m_response.size()<<std::endl;
	}
}

void	Response::clear()
{
	m_response.clear();
	m_body_size = 0;
	m_content_type.clear();
	m_error_page.clear();
}
