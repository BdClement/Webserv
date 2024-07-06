/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handler.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/16 12:55:53 by clbernar          #+#    #+#             */
/*   Updated: 2024/07/06 15:28:26 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Handler.hpp"
#include "ConfigParser.hpp"

struct clearFromHanlder global = {NULL, NULL, NULL, 0};

/***********************************************************************************************************
 *                                                                                                         *
 *                                           CANONICAL FORM CLASS                                          *
 *                                                                                                         *
 ***********************************************************************************************************/

Handler::Handler() : m_config(), m_listen_connection(), m_http_connection(), epoll_fd(0)
{
	extern clearFromHanlder global;
	global.global_config = &(this->m_config);
	global.global_listen_connection = &(this->m_listen_connection);
	global.global_http_connection = &(this->m_http_connection);
	global.global_epoll_fd = &(this->epoll_fd);
}

Handler::Handler(Handler const& asign)
{
	(void)asign;
}

Handler::~Handler()
{
	// std::cout<<"Handler destructor called\n"<<std::endl;
}

Handler& Handler::operator=(Handler const & equal)
{
	if (this != &equal)
	{

	}
	return *this;
}

/***********************************************************************************************************
 *                                                                                                         *
 *                                                 UTILS                                                   *
 *                                                                                                         *
 ***********************************************************************************************************/

// This function returns index of Connection object that have detected an event
int		Handler::recoverIndexConnection(int const socket) const
{
	for (int i = 0; i < (int)m_http_connection.size(); ++i)
	{
		if (m_http_connection[i].socket == socket || m_http_connection[i].request.m_pipe.pipe_stdin[1] == socket ||
			m_http_connection[i].request.m_pipe.pipe_stdout[0] == socket)
			return i;
	}
	return -1;
}

// This function checks if a socket is a listening socket
bool	Handler::isListenningConnection(int const socket)
{
	std::vector<Connection>::iterator it = find_if(m_listen_connection.begin(), m_listen_connection.end(), CompareSocket(socket));
	if (it != m_listen_connection.end())
	{
		// std::cout<<"\t"<<socket<<" is a listenning socket"<<std::endl;
		return true;
	}
	else
	{
		// std::cout<<"\t"<<socket<<" is not a listenning socket"<<std::endl;
		return false;
	}
}

// This function checks if a socket has already been created for an interface
bool	Handler::interfaceAlreadyExist(std::vector<ServerConfig>::iterator const& toFind)
{
	for (std::vector<ServerConfig>::iterator it = m_config.begin(); it != m_config.end(); ++it)
	{
		if (it == toFind)
			return false;
		if (it->_host == toFind->_host && it->_port == toFind->_port)
		{
			PRINT_RED("Socket not created")<<" interface ["<<toFind->_host<<":"<<toFind->_port<<"]. This interface already has a dedicated socket."<<std::endl;
			return true;
		}
	}
	return false;
}

/***********************************************************************************************************
 *                                                                                                         *
 *                                            PARSING CONFIG FILE                                          *
 *                                                                                                         *
 ***********************************************************************************************************/

bool	Handler::initConfig(char * arg)
{
	ConfigParser servers;
	std::string configFile;
	if (arg == NULL)
		configFile = "test/conf/file.conf";
	else
		configFile = arg;
	try
	{
		servers.createServers(configFile);
		// servers.printServers();
		m_config = servers._servers;
	}
	catch(const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return false;
	}
	return true;
}

/***********************************************************************************************************
 *                                                                                                         *
 *                                           SERVER INITIALISATION                                         *
 *                                                                                                         *
 ***********************************************************************************************************/

 // Les ports en dessous de 1024 ne peuvent pas etre bind() car l'OS a une politique de securite
// concernant ses ports la qui sont consideres comme reserves ou bien connus.
// This function create all the listening sockets and bind them to interfaces from configuration file
// And add them to epoll
void	Handler::initServer()
{
	std::cout<<"************************************************************\n\n*                        WEBSERV                           *\n\n************************************************************\n"<<std::endl;
	this->epoll_fd = epoll_create1(0);
	int i = 0;
	if (this->epoll_fd == -1)
	{
		PRINT_RED("Epoll_fd creation failed : ")<<strerror(errno)<<std::endl;
		exit(errno);
	}
	std::cout<<"\n*****************   SERVER INITIALIATION   *****************\n"<<std::endl;
	for (std::vector<ServerConfig>::iterator it = m_config.begin(); it != m_config.end(); ++it)
	{
	            //    ************************************************************
		std::cout<<"\n\n==== ServerConfig Block "<<(i++ + 1)<<" ===="<<std::endl;
		if (!interfaceAlreadyExist(it))
		{
			Connection	new_connection;
			if (!this->initListenConnection(it, new_connection))
				continue;
			//BIND
			std::cout<<"--- Binding socket to interface ---"<<std::endl;
			if (bind(new_connection.socket, (struct sockaddr *)&(new_connection.interface), sizeof(new_connection.interface)) == -1)
			{
				PRINT_RED("\tFail")<<" : Socket "<<new_connection.socket<<" to interface "<<it->_host<<":"<<it->_port<<" : "<<strerror(errno)<<std::endl;
				if (close(new_connection.socket) == -1)
					PRINT_RED("Closing socket failed : ")<<strerror(errno)<<std::endl;
				continue;
			}
			else
				PRINT_GREEN("\tSuccess")<<std::endl;
			//LISTEN
			std::cout<<"--- Listenning socket ---"<<std::endl;
			if (listen(new_connection.socket, 20) == -1)
			{
				PRINT_RED("\tFail : ")<<strerror(errno)<<std::endl;
				if (close(new_connection.socket) == -1)
					PRINT_RED("Closing socket failed : ")<<strerror(errno)<<std::endl;
				continue;
			}
			else
				PRINT_GREEN("\tSuccess")<<std::endl;
			//Ajout de la socket d'ecoute au vecteur
			m_listen_connection.push_back(new_connection);
			std::cout<<"--- Adding socket to epoll ---"<<std::endl;
			if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, new_connection.socket, &(new_connection.event)) == -1)
				PRINT_RED("\tFail : ")<<strerror(errno)<<std::endl;
			else
				PRINT_GREEN("\tSuccess")<<" : socket "<<new_connection.socket<<" to interface ["<<it->_host<<":"<<it->_port<<"]"<<std::endl;
		}
	}
	PRINT_GREEN("Initialisation finished")<<". "<<this->m_listen_connection.size()<<" listenning socket(s) created in total.\n"<<std::endl;
}

// This functions fills structures sockaddr_in and epoll_event of Connection object
bool	Handler::initListenConnection(std::vector<ServerConfig>::iterator & it, Connection & new_connection)
{
	// SOCKET
	std::cout<<"--- Socket creation ---"<<std::endl;
	new_connection.socket = socket(AF_INET, SOCK_STREAM, 0);
	if (new_connection.socket == -1)
	{
		PRINT_RED("\tFail")<<" : "<<strerror(errno)<<std::endl;
		return false;
	}
	else
		PRINT_GREEN("\tSuccess")<<std::endl;
	if (fcntl(new_connection.socket, F_SETFL, O_NONBLOCK) < 0)
	{
		PRINT_RED("Error O_NONBLOCK failed on socket")<<" : Listenning socket "<<new_connection.socket<<" closed"<<std::endl;
		close(new_connection.socket);
		return false;
	}
	std::cout<<"--- Connection object initialization ---"<<std::endl;
	PRINT_GREEN("\tSuccess [Interface and event]")<<std::endl;
	// INTERFACE
	new_connection.interface.sin_family = AF_INET;
	new_connection.interface.sin_port = htons(it->_port);
	if (it->_host == "0.0.0.0")
	{
		std::cout<<"C'est bien rentre mon boug"<<std::endl;
		new_connection.interface.sin_addr.s_addr = INADDR_ANY;
	}
	else
		new_connection.interface.sin_addr.s_addr = htonl(convertAddr(it->_host));
	// //EVENT
	new_connection.event.events = EPOLLIN;
	new_connection.event.data.fd = new_connection.socket;
	return true;
}

/***********************************************************************************************************
 *                                                                                                         *
 *                                             LAUNCHING SERVER                                            *
 *                                                                                                         *
 ***********************************************************************************************************/

// Les connections TCP ne sont pas directement free (meme avec un close), la socket
// devient en mode TIME_WAIT pour donner du temps a la socket de ferme proprement
// apres un certain temps l'OS libere l'adresse
// -Boucle principale a mettre en place
// This function is the main monitoring loop to detect event and handle them
void	Handler::launchServer()
{
	std::cout<<"\n*****************       SERVER LAUNCH      *****************\n"<<std::endl;
	while (1)
	{
		struct epoll_event	events[MAX_EVENTS];// Taille a debattre & memset ?
		int	num_event = epoll_wait(epoll_fd, events, MAX_EVENTS, 5);
		if (num_event == -1)
			PRINT_RED("Waiting event failed : ")<<strerror(errno)<<std::endl;
		else
		{
			for (int i = 0; i < num_event; ++i)
			{
				// LOGIQUE D'ACCEPTATION DE CONNEXION ENTRANTE
				if (isListenningConnection(events[i].data.fd))
					acceptIncomingConnection(events[i].data.fd);
				else// LOGIQUE DE GESTION DE SOCKET DE COMMUNICATION
				{
					int	index = recoverIndexConnection(events[i].data.fd);
					if (index == -1)
						continue;
					// Logique de gestion des evenements
					if (events[i].events & EPOLLIN)
						epollinEvent(events[i], index);
					else if (events[i].events & EPOLLOUT)
						epolloutEvent(events[i], index);
					else if (events[i].events & EPOLLERR)
						handlingEpollerrEvent(m_http_connection[index]);
					else if (events[i].events & EPOLLHUP)
						epollhupEvent(events[i], index);
				}
			}
			handlingKeepAlive();
			if (num_event > MAX_EVENTS)
				std::cout<<"Warning : more events than MAX_EVENTS have been detected. It could affect server's performance"<<std::endl;
		}
	}
}

// This function accept incoming connection, store the socket in a dedicated Connection object
// and add the socket to epoll
void	Handler::acceptIncomingConnection(int const socket)
{
	Connection new_connection;
	socklen_t	client_len = sizeof(new_connection.interface);
	new_connection.socket = accept(socket, (struct sockaddr *)&(new_connection.interface), &client_len);
	if (new_connection.socket == -1)
	{
		PRINT_RED("Failed ")<<": "<<strerror(errno)<<std::endl;
		return;
	}
	else
	{
		if (fcntl(new_connection.socket, F_SETFL, O_NONBLOCK) < 0)
		{
			closeAndRmConnection(new_connection);
			return ;
		}
		new_connection.event.events = EPOLLIN | EPOLLERR | EPOLLHUP;
		new_connection.event.data.fd = new_connection.socket;
		if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, new_connection.socket, &(new_connection.event)) == -1)
			PRINT_RED("Failed ")<<": "<<strerror(errno)<<" ]"<<std::endl;
		if(!new_connection.setAddrPort())
			return;
		PRINT_GREEN("New connection accepted")<<" : socket "<<new_connection.socket<<"[ "<<new_connection.addr<<":"<<new_connection.port<<"]"<<std::endl;
		m_http_connection.push_back(new_connection);
	}
}

/***********************************************EPOLLIN********************************************************/

void	Handler::epollinEvent(struct epoll_event & events, int index)
{
	if (events.data.fd != m_http_connection[index].socket)
		handlingCgiEpollin(m_http_connection[index], events.data.fd);
	else
		handlingEpollinEvent(m_http_connection[index]);
}

// This function reads request and treat it with it Connection object
// It creates an http Response and make EPOLLOUT event on the socket as the response is ready
void	Handler::handlingEpollinEvent(Connection & connection)
{
	connection.last_active_time = time(NULL);
	unsigned char	buffer[BUFFER_SIZE];
	memset(buffer, '\0', sizeof(buffer));
	int bytes_read = recv(connection.socket, buffer, sizeof(buffer), 0);
	if (bytes_read <= 0)
	{
		if (bytes_read == 0)
			std::cout<<"  Client closed connection socket ["<<connection.socket<<"]"<<std::endl;
		closeAndRmConnection(connection);
	}
	else
	{
		//PARSE REQUEST
		if (connection.request.m_error_code == 0)
		{
			connection.request.m_read.insert(connection.request.m_read.end(), buffer, buffer + bytes_read);
			connection.request.parseRequest();
		}
		// END OF PARSING
		if (connection.request.m_ready)
		{
			addEpollout(connection);
			// PRINT_RED("TEST m_cgi = ")<<connection.request.m_cgi<<" on socket "<<connection.socket<<std::endl;
		}
	}
}

void	Handler::handlingCgiEpollin(Connection & connection, int &fd)
{
	// PRINT_GREEN("handlingCgiEpollin called on fd :")<<fd<<std::endl;
	// PRINT_RED("TEST m_cgi = ")<<connection.request.m_cgi<<" on socket "<<connection.socket<<std::endl;
	char buffer[60000];
	ssize_t bytes_read = read(fd, buffer, 60000);
	if (bytes_read == -1)
	{
		// PRINT_RED("Error reading pipe CGI : ")<<fd<<std::endl;
		close(fd);
		connection.request.m_pipe.pipe_stdout[0] = 0;
		kill(connection.request.m_pipe.m_pid, SIGINT);// waitpid ?
		// kill fantome ?
		connection.request.m_error_code = 500;
		addEpollout(connection);
		return ;
	}
	if (bytes_read == 0)
	{
		// PRINT_GREEN("EOF a ete atteint : toute la reponse a ete recuperer")<<std::endl;
		// Check Content-Length par exmple
		connection.request.m_response_code = 200;
		close(fd);
		connection.request.m_pipe.pipe_stdout[0] = 0;
		addEpollout(connection);
		return ;
	}
	connection.response.m_response.insert(connection.response.m_response.end(), buffer, buffer + bytes_read);
	// std::cout<<"TEST size de la reponse : "<<connection.response.m_response.size()<<std::endl;
	// std::cout<<"\nResponse received :\n["<<std::endl;
	// for (std::vector<unsigned char>::iterator it = connection.response.m_response.begin(); it != connection.response.m_response.end(); ++it)// AFFICHAGE DE TEST
	// 		std::cout<<*it;
	// std::cout<<std::endl;
	// PRINT_RED("TEST m_cgi = ")<<connection.request.m_cgi<<" on socket "<<connection.socket<<std::endl;
}

/***********************************************EPOLLOUT*******************************************************/

void	Handler::epolloutEvent(struct epoll_event & events, int index)
{
	if (events.data.fd != m_http_connection[index].socket)
		handlingCgiEpollout(m_http_connection[index], events.data.fd);
	else
		handlingEpolloutEvent(m_http_connection[index]);
}

int	Handler::findServerBlock(Connection & connection)
{
	// Extraction des donnees de la request
	std::string	request_host, request_port;
	std::size_t pos = connection.request.m_headers["Host:"].find(":");
	request_host = connection.request.m_headers["Host:"].substr(0, pos);
	if (pos != std::string::npos)// : present dans le header Host
		request_port = connection.request.m_headers["Host:"].substr(pos + 1);
	// Comparaison port de connection avec port de la requete
	std::ostringstream oss;
	oss<<connection.port;
	std::string	connection_port = oss.str();
	if (!request_port.empty() && connection_port != request_port)// Check que le request_port correspond au connection_port
	{
		connection.request.m_error_code = 400;
		return -1;
	}
	// std::cout<<"TEST affichage oss "<<connection_port<<" request_host "<< request_host<<" request_port "<<request_port<<std::endl;
	int	first_match = -1;
	for (int i = 0; i < (int)m_config.size(); ++i)
	{
		// Si l'interface connection correspond a l'interface bloc server
		if (connection.addr == m_config[i]._host && connection.port == m_config[i]._port)
		{
			if (first_match == -1)
				first_match = i;
			if (request_host == m_config[i]._serverName)
			{
				// PRINT_GREEN("Le bloc selectionner est le bloc : ")<<i<<std::endl;
				return i;
			}
		}
	}
	// PRINT_GREEN("Le bloc selectionner est le bloc : ")<<first_match<<std::endl;
	return first_match;
}

int	Handler::findLocationBlock(ServerConfig & serverBlock, std::string & uri)
{
	// PRINT_RED("TEST affichage findLocationBlock uri : ")<<uri<<std::endl;
	if (serverBlock._locations.size() == 0)
		return -1;
	int	loc_index = -1;
	std::size_t longest_match = 0;
	std::string toTest;
	for (int i = 0; i < (int)serverBlock._locations.size(); ++i)
	{
		if (uri.size() != 0 && serverBlock._locations[i]._pathLoc.size() != 0
			&& serverBlock._locations[i]._pathLoc[serverBlock._locations[i]._pathLoc.size() - 1] == '/' && uri[uri.size() - 1] != '/')
			toTest = uri + "/";
		else
			toTest = uri;
		if (toTest.find(serverBlock._locations[i]._pathLoc) == 0)
		{
			if (serverBlock._locations[i]._pathLoc.length() > longest_match)
			{
				longest_match = serverBlock._locations[i]._pathLoc.length();
				loc_index = i;
			}
		}
	}
	// uri.erase(longest_match);
	if (serverBlock._locations[loc_index]._cgiExt.empty())
	{
		if (uri.size() < serverBlock._locations[loc_index]._pathLoc.size())
			uri = "";
		else
			uri.erase(0, longest_match);
		if (uri[0] == '/')
			uri.erase(0, 1);
	}
	// PRINT_RED("Uri devient ")<<uri<<std::endl;
	// PRINT_GREEN("Affichage resultat de findLocationBlock : ")<<loc_index<<std::endl;
	return loc_index;
}

// Attention a la gestion des requetes fragmentees
// Clean connection en cas d'erreur d'envoi de la requete
// This Function sends the response on the socket and remove EPOLLOUT event on this socket as the reponse has been sent
void	Handler::handlingEpolloutEvent(Connection & connection)
{
	// PRINT_GREEN("handlingEpolloutEvent called")<<std::endl;
	// PRINT_RED("TEST m_cgi = ")<<connection.request.m_cgi<<" on socket "<<connection.socket<<std::endl;
	// std::cout<<"\nRequest received :\n["<<std::endl;
	// for (std::vector<unsigned char>::iterator it = connection.request.m_read.begin(); it != connection.request.m_read.end(); ++it)// AFFICHAGE DE TEST
	// 		std::cout<<*it;
	// std::cout<<std::endl;
	int server_index = 0;
	int	location_index = 0;
	// PRINT_GREEN("TEST EPOLLOUT 1")<<std::endl;
	if (!connection.request.m_cgi)
	{
		// std::cout<<"  Event EPOLLOUT detected on socket "<<connection.socket<<std::endl;
		server_index = findServerBlock(connection);
		// PRINT_GREEN("TEST EPOLLOUT 1 bis")<<std::endl;
		location_index = findLocationBlock(m_config[server_index], connection.request.m_uri);
		// PRINT_GREEN("TEST EPOLLOUT 1 ter")<<std::endl;
		std::cout<<"Index server : "<<server_index<<" Index location : "<<location_index<<std::endl;
		connection.request.setConfig(m_config[server_index], location_index);
		if (connection.request.m_error_code == 0)
			connection.request.checkBody();
		// std::cout<<"[ Logique de traitement de requete ]"<<std::endl;
		// PRINT_GREEN("TEST EPOLLOUT 2")<<std::endl;
		if (connection.request.m_error_code == 0)
			connection.request.processRequest(m_config[server_index], location_index, connection.response);
		// PRINT_GREEN("TEST EPOLLOUT 3")<<std::endl;
		if (connection.request.m_error_code == 0 && connection.request.m_cgi)
		{
			setPipeMonitoring(connection);
			rmEpollout(connection);
			// PRINT_RED("TEST m_cgi = ")<<connection.request.m_cgi<<" on socket "<<connection.socket<<std::endl;
			return ;
		}
	}
	// PRINT_GREEN("TEST EPOLLOUT 4")<<std::endl;
	// std::cout<<"[ Logique de generation de reponse HTTP ]"<<std::endl;
	if (connection.request.m_cgi && connection.request.m_error_code == 0)
		connection.response.generateStatusLine(connection.request);
	else
		connection.response.generateResponse(connection.request, m_config[server_index], location_index);
	// SENDING RESPONSE
	const unsigned char* rep = &(connection.response.m_response[0]);
	// PRINT_GREEN("Contenu de ce qui a ete envoye = ")<<std::endl;
	// for (std::vector<unsigned char>::iterator it = connection.response.m_response.begin(); it != connection.response.m_response.end(); ++it)
	// 	std::cout<<*it;
	// std::cout<<std::endl;
	if (send(connection.socket, rep, connection.response.m_response.size(), 0) == -1)
	{
		PRINT_RED("  ERROR couldn't send response")<<std::endl;
		closeAndRmConnection(connection);
	}
	else
		PRINT_GREEN(" Connection socket ")<<connection.socket<<" [ "<<connection.addr<<":"<<connection.port<<"]"<<std::endl;
	rmEpollout(connection);
	// PRINT_RED("TEST m_cgi = ")<<connection.request.m_cgi<<" on socket "<<connection.socket<<std::endl;
	// Si !keep alive dans la requete  => close connection
	if (!connection.request.isKeepAlive() || connection.request.m_redirection)
		closeAndRmConnection(connection);
	else // A conditionner avec requetes fragmentees
	{
		connection.request.clear();
		connection.response.clear();
	}
}

// Forcement requete POST
void	Handler::handlingCgiEpollout(Connection & connection, int &fd)
{
	PRINT_GREEN("handlingCgiEpollout called on fd :")<<fd<<std::endl;
	// PRINT_RED("TEST m_cgi = ")<<connection.request.m_cgi<<" on socket "<<connection.socket<<std::endl;
	////// Determiner le nb de carcateres a lire /////////////
	int to_read;// = 100;
	if (connection.request.m_body_pos + connection.request.m_pipe.m_bytes_sent + 60000 > connection.request.m_read.size())
		to_read = connection.request.m_read.size() - (connection.request.m_body_pos + connection.request.m_pipe.m_bytes_sent);
	else
		to_read = 60000;
	//////////////////////////////////////////////////////////
	// PRINT_GREEN("Envoi du body au script CGI")<<std::endl;
	// std::cout<<"TEST : "<<&(connection.request.m_read[0])<<std::endl;//connection.request.m_body_pos + connection.request.m_pipe.m_bytes_sent
	// std::cout<<"TEST : body_pos = "<<connection.request.m_body_pos<<" Size de m_read = "<<connection.request.m_read.size()<<" bytes_sent = "<<connection.request.m_pipe.m_bytes_sent<<std::endl;
	ssize_t bytes_written = write(fd, &(connection.request.m_read[connection.request.m_body_pos + connection.request.m_pipe.m_bytes_sent]), to_read);
	connection.request.m_pipe.m_bytes_sent += bytes_written;
	if (bytes_written == -1)
	{
		PRINT_RED("Error writing pipe : ")<< fd<<std::endl;
		kill(connection.request.m_pipe.m_pid, SIGINT);// waitpid ?
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
		connection.request.m_pipe.pipe_stdin[1] = 0;
		// fd = 0;
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, connection.request.m_pipe.pipe_stdout[0], NULL);
		close(connection.request.m_pipe.pipe_stdout[0]);
		connection.request.m_pipe.pipe_stdout[0] = 0;
		connection.request.m_error_code = 500;
		addEpollout(connection);
		return ;
		// close le child process pour eviter les fantomes
	}
	// std::cout<<"TEST : bytes_written = "<<bytes_written<<std::endl;
	// std::cout<<"TEST : body_pos + bytes_sent = "<<connection.request.m_body_pos + connection.request.m_pipe.m_bytes_sent<<" contre size = "<<connection.request.m_read.size()<<std::endl;
	if (connection.request.m_body_pos + connection.request.m_pipe.m_bytes_sent == connection.request.m_read.size())
	{
		// PRINT_GREEN("L'ensemble du body a ete ecrit sur le pipe")<<std::endl;
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
		connection.request.m_pipe.pipe_stdin[1] = 0;
		// fd = 0;
	}
}

/***********************************************EPOLLHUP*******************************************************/

void	Handler::epollhupEvent(struct epoll_event & events, int index)
{
	// PRINT_RED("EPOLLHUP detecte sur le fd ou socket : ")<<events.data.fd<<std::endl;
	if (events.data.fd == m_http_connection[index].socket)
	{
		PRINT_RED("Connection closed by client on socket ")<<m_http_connection[index].socket<<std::endl;
		closeAndRmConnection(m_http_connection[index]);
	}
	else
		handlingCgiEpollhup(m_http_connection[index]);
}

void	Handler::handlingCgiEpollhup(Connection & connection)
{
	// PRINT_GREEN("handlingCgiEpollhup called on fd :")<<std::endl;
	// PRINT_RED("TEST m_cgi = ")<<connection.request.m_cgi<<" on socket "<<connection.socket<<std::endl;
	int status;
	pid_t result = waitpid(connection.request.m_pipe.m_pid, &status, WNOHANG);
	// if (result == 0)
		// PRINT_GREEN("Le process est toujours en cours d'execution : ")<<connection.request.m_pipe.m_pid<<std::endl;
	/*else*/ if (result == -1)
		PRINT_RED("Error de waitpid")<<std::endl;
	else
	{
		if (WIFEXITED(status))// Si le child a termine son execution
		{
			// PRINT_GREEN("Le child process s'est termine")<<std::endl;
			if (WEXITSTATUS(status) == SIGINT) // Si le child a rencontre une erreur
			{
				// PRINT_GREEN("Le child a quitte grace a SIGINT, une erreur doit etre retorune")<<std::endl;
				if (connection.request.m_method == "POST" && connection.request.m_pipe.pipe_stdin[1] != 0)
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, connection.request.m_pipe.pipe_stdin[1], NULL);
				epoll_ctl(epoll_fd, EPOLL_CTL_DEL, connection.request.m_pipe.pipe_stdout[0], NULL);// Premier bout de pipe
				// epoll_ctl(epoll_fd, EPOLL_CTL_DEL, connection.request.m_pipe.pipe_stdin[1], NULL);// Deuxieme bout de pipe
				connection.request.m_pipe.clear();
				connection.request.m_error_code = 500;
				addEpollout(connection);
				return ;
			}
			else// Si le child a execute le script correctement
			{
				// PRINT_GREEN("EOF a ete atteint : toute la reponse a ete recuperer")<<std::endl;
				// Check Content-Length par exmple
				epoll_ctl(epoll_fd, EPOLL_CTL_DEL, connection.request.m_pipe.pipe_stdout[0], NULL);// Premier bout de pipe
				epoll_ctl(epoll_fd, EPOLL_CTL_DEL, connection.request.m_pipe.pipe_stdin[1], NULL);// Deuxieme bout de pipe
				connection.request.m_pipe.clear();
				connection.request.m_response_code = 200;
				addEpollout(connection);
				return ;
			}
		}
	}
}

/***********************************************EPOLLERR*******************************************************/

// Clean connection en cas d'event EPOLLERR detecte : En fonction de l'erreur ou tout le temps ?
void	Handler::handlingEpollerrEvent(Connection & connection)
{
	// std::cout<<"\tEvent EPOLLERR detected on socket "<<connection.socket<<" : "<<strerror(errno)<<std::endl;
	// Gestion d'erreur
	closeAndRmConnection(connection);// A checker si c'est necessaire ici : Choix
}

/*******************************************EVENT MONITORING***************************************************/

void	Handler::addEpollout(Connection & connection)
{
	// PRINT_GREEN("addEpollout called")<<std::endl;
	if (!(connection.event.events & EPOLLOUT))
	{
		// |= "ou" binaire avec assignation. Permet de combiner les bits deja presents au bit EPOLLOUT que l'on ajoute
		connection.event.events |= EPOLLOUT;
		// On declare qu'on est pret a repondre sur la socket concernee (A voir avec les requetes fragmentees)
		/*if (*/epoll_ctl(this->epoll_fd, EPOLL_CTL_MOD, connection.socket, &(connection.event)); /*== -1)*/
			// PRINT_RED("  Adding EPOLLOUT to watched event failed :")<<" socket "<<connection.socket<<std::endl;// Fermer la connetion ?
		// else
			// PRINT_GREEN("  Adding EPOLLOUT to watched event")<<" socket "<<connection.socket<<std::endl;
	}
}

void	Handler::rmEpollout(Connection & connection)
{
	// PRINT_GREEN("rmEpollout called")<<std::endl;
	// &= ~ "et" binaire avec negation de EPOLLOUT et assignation
	// Cela permet de supprimer le bit EPOLLOUT en inversant sa valeur
	connection.event.events &= ~EPOLLOUT;
	//On declare qu'on ne surveille plus EPOLLOUT puisqu'on a envoye notre reponse
	if (epoll_ctl(this->epoll_fd, EPOLL_CTL_MOD, connection.socket, &(connection.event)) == -1)
	{
		// PRINT_RED("  Removing EPOLLOUT to watched event failed :")<<" socket "<<connection.socket<<"]"<<std::endl;
		closeAndRmConnection(connection);//Fermeture car surement boucle infinie de EPOLLOUT
	}
	// else
		// PRINT_GREEN("  Removing EPOLLOUT to watched event")<<" socket "<<connection.socket<<"]"<<std::endl;
}

void	Handler::setPipeMonitoring(Connection & connection)
{
	// PRINT_GREEN("setPipeMonitoring called")<<std::endl;
	struct epoll_event stdin;
	struct epoll_event stdout;
	stdin.events = EPOLLOUT;
	stdin.data.fd = connection.request.m_pipe.pipe_stdin[1];
	stdout.events = EPOLLIN;
	stdout.data.fd = connection.request.m_pipe.pipe_stdout[0];
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connection.request.m_pipe.pipe_stdout[0], &stdout) == -1)// Ici Probleme !
	{
		// PRINT_RED("Ading pipe to epoll failed")<<std::endl;
		connection.request.m_pipe.pipe_stdin[0] = 0;
		connection.request.m_pipe.pipe_stdout[1] = 0;
		connection.request.m_pipe.clear();
		connection.request.m_error_code = 500;
		// PRINT_RED("Mise a false de m_cgi")<<std::endl;
		connection.request.m_cgi = false;
		return ;
	}
	if (connection.request.m_method == "POST")
	{
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connection.request.m_pipe.pipe_stdin[1], &stdin) == -1)
		{
			// PRINT_RED("Ading pipe to epoll failed")<<std::endl;
			connection.request.m_pipe.pipe_stdin[0] = 0;
			connection.request.m_pipe.pipe_stdout[1] = 0;
			connection.request.m_pipe.clear();
			connection.request.m_error_code = 500;
			// PRINT_RED("Mise a false de m_cgi")<<std::endl;
			connection.request.m_cgi = false;
		}
	}
	// PRINT_RED("TEST m_cgi = ")<<connection.request.m_cgi<<" on socket "<<connection.socket<<std::endl;
}

/***********************************************************************************************************
 *                                                                                                         *
 *                                              CLEANING SERVER                                            *
 *                                                                                                         *
 ***********************************************************************************************************/

// Je n'utilise pas erase pour ne pas a avoir a parcourir a chaque fois mon vecteur
// Comme l'ordre dans mon vecteur n'est pas important, je place l'element que je souhaite
// Supprimer en dernier puis je le supprime
// This function close socket and remove Connection object in m_http_connection vector
void	Handler::closeAndRmConnection(Connection & connection)
{
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, connection.socket, NULL);
	if (close(connection.socket) == -1)
		PRINT_RED("\tClosing connection failed ")<<"on socket "<<connection.socket<<" : "<<strerror(errno)<<std::endl;
	else
		std::cout<<"\tConnection socket "<<connection.socket<<" closed"<<std::endl;
	std::swap(connection, m_http_connection.back());
	m_http_connection.pop_back();
}

// This function checks if some HTTP Connection has been kept open for too long
// If so, it closes them and remove Connection object
void	Handler::handlingKeepAlive()
{
	// int count = 0;
	for (std::vector<Connection>::iterator it = m_http_connection.begin(); it != m_http_connection.end();)
	{
		// if (it->request.m_cgi)
		// 	count++;
		if (difftime(time(NULL), it->last_active_time) > TIMEOUT)
		{
			// PRINT_RED("TEST1 : ")<<it->request.m_cgi<<" on socket "<<it->socket<<" m_uri = "<<(it->request).m_uri<<std::endl;
			if (it->request.m_cgi)
			{
				PRINT_RED("Kill du child process")<<std::endl;
				kill(it->request.m_pipe.m_pid, SIGINT);
				if (it->request.m_method == "POST" && it->request.m_pipe.pipe_stdin[1] != 0)
					epoll_ctl(epoll_fd, EPOLL_CTL_DEL, it->request.m_pipe.pipe_stdin[1], NULL);
				epoll_ctl(epoll_fd, EPOLL_CTL_DEL, it->request.m_pipe.pipe_stdout[0], NULL);
				std::string rep = "HTTP/1.1 500 Internal Servor Error";
				if (send(it->socket, rep.c_str(), rep.size(), 0) == -1)
					PRINT_RED("  ERROR couldnt send response")<<std::endl;
				// else
				// 	PRINT_GREEN(" Response sent")<<std::endl;
				int status;
				waitpid(it->request.m_pipe.m_pid, &status, 0);// Pb du Broken pipe
				// PRINT_RED("TEST pipe stdout[0] = ")<<it->request.m_pipe.pipe_stdout[0]<<std::endl;
				it->request.m_pipe.clear();
			}
			// PRINT_RED("TEST2")<<std::endl;
			// std::cout<<difftime(time(NULL), it->last_active_time)<<" > "<<TIMEOUT<<std::endl;
			std::vector<Connection>::iterator next = it;
			++next;
			/////////////
			epoll_ctl(epoll_fd, EPOLL_CTL_DEL, it->socket, NULL);
			if (close(it->socket) == -1)
				PRINT_RED("\tClosing connection failed ")<<"on socket "<<it->socket<<" : "<<strerror(errno)<<std::endl;
			else
				PRINT_GREEN("Connection socket closed : ")<<it->socket<<" | Kept alive 30 sec"<<std::endl;
			it = m_http_connection.erase(it);
		}
		else
			++it;
	}
	// PRINT_RED("Nombre de requete CGI : ")<<count<<std::endl;
}

/***********************************************************************************************************
 *                                                                                                         *
 *                                                 Hors-Sujet                                              *
 *                                                                                                         *
 ***********************************************************************************************************/



// // This functions convert a string address IPv4 in unsigned long to put it in a sockaddr_in struct
// // Same role as inet_pton() function that converts text to binary form (that we can't use in the project)
// // It uses std::istringstream to split the string with getline from each "."
// // It stocks in the atoi conversion of each part in ip_bytes (unsigned char tab)
// // And it shitfs the bytes to reorganise in unsigned long format
unsigned long	convertAddr(std::string to_convert)
{
	// CHECK mauvais format de string  => return 0
	std::istringstream	iss(to_convert);
	std::string	token;
	unsigned char	ip_bytes[4];
	int i = 0;

	while (std::getline(iss, token, '.'))
		ip_bytes[i++] = atoi(token.c_str());

	// for (int j = 0; j < 4; ++j)
	// 	std::cout<<ip_bytes[j]<<" ";
	// std::cout<<std::endl;

	unsigned long	ip_addr =
		(ip_bytes[0] << 24) |
		(ip_bytes[1] << 16) |
		(ip_bytes[2] << 8) |
		ip_bytes[3];
	return ip_addr;
}

// This function convert an unsigned long in std::string to recover and compare Connection's adress
std::string convertAddrBack(unsigned long ip_addr)
{
	unsigned long	addr = ntohl(ip_addr);
	unsigned char ip_bytes[4];
	ip_bytes[0] = (addr >> 24) & 0xFF;
	ip_bytes[1] = (addr >> 16) & 0xFF;
	ip_bytes[2] = (addr >> 8) & 0xFF;
	ip_bytes[3] = addr & 0xFF;
	std::ostringstream oss;
	oss << static_cast<int>(ip_bytes[0]) << "."
		<< static_cast<int>(ip_bytes[1]) << "."
		<< static_cast<int>(ip_bytes[2]) << "."
		<< static_cast<int>(ip_bytes[3]);

	return oss.str();
}


// EXPLICATION 1 : La gestion de memoire des vector semble complexe. Apres avoir close toutes les sockets
// J'avais des still recheable (alors que je n'ai rien alloue dynamiquement) qui provenait donc de mes vector
// Pour liberer la memoire allouee par le vecteur lui meme, la solution etait d'utiliser shrink_to_fit() pour
// l'espace non necessaire apres avoir clear le vector mais cette methode des vectors est issue de C++11.
// Pour regler ce soucis je suis passer par un vector temporaire en utilisant "std::vector<Config>()" qui permet
// de creer un objet temporaire qui ne sera donc pas stocker dans une variable.
// Je swap ce vector vide avec mon vector qui se retrouve donc dans mon vector temporaire
// Le destructeur du vector est alors appele sur mon vector et ma variable globale contient desormais un vector vide
// EXPLICATION 2 : Par souci de simplicite et parce que le but du projet est bien loin de la gestion des signaux,
// j'ai choisi de gerer les signaux en dehors de ma classe Handler en passant une structure globale pour avoir acces
// aux donnees a liberer ou a close dans mon gestionnaire de signal.
// La solution plus propre aurait ete de rendre ma classe Handler Singleton (design pattern ou patron de conception)
// c'est a dire la rendre instanciable qu'une seule fois comme ce qui est amene a etre le cas.
// Cette methode implique d'avoir un constructeur privee, une methode d'acces statique qui renvoie l'instance unique
// de la classe, une instance statique cree a l'interieur de la methode d'acces statique et un pointeur vers l'instance
// unique a travers lequel j'aurais pu acceder a mes donnees dans ma gestion des signaux.
// Cela permet d'appeler la methode statique d'acces depuis n'importe ou pour avoir acces a mes elements.
// Cette methode aurait aussi pu s'utiliser sur une autre classe a l'interieur de ma classe Handler.
void	signalHandler(int signal_num)
{
	extern clearFromHanlder global;

	if (signal_num == SIGINT)
		std::cout<<"Signal SIGINT received. Cleaning in progress..."<<std::endl;
	else if (signal_num == SIGQUIT)
		std::cout<<"Signal SIGQUIT received. Cleaning in progress..."<<std::endl;
	// if (global.cgi_strings != NULL)
	// {
	// 	global.cgi_strings->clear();
	// 	// std::vector<std::string>().swap(*global.cgi_strings);
	// }
	//VECTOR CONFIG
	std::vector<ServerConfig>().swap(*global.global_config);
	//VECTOR LISTENNING SOCKET
	std::cout<<"Cleaning listenning sockets ..."<<std::endl;
	std::for_each(global.global_listen_connection->begin(), global.global_listen_connection->end(), &closeSocket);
	std::vector<Connection>().swap(*global.global_listen_connection);
	//VECTOR HTTP SOCKET
	std::cout<<"Cleaning http communication sockets ...\n"<<std::endl;
	std::for_each(global.global_http_connection->begin(), global.global_http_connection->end(), &closeSocket);
	std::vector<Connection>().swap(*global.global_http_connection);
	//VECTOR VECTOR READING FROM SOCKET A garder en tete si leak ?
	// EPOLL
	if (close(*(global.global_epoll_fd)) == -1)
		PRINT_RED("Closing epoll_fd failed ")<<strerror(errno)<<std::endl;
	else
		std::cout<<"Epoll_fd closed"<<std::endl;
	std::cout<<"Exit avec "<<signal_num<<" Et SIGINT = "<<SIGINT<<std::endl;
	exit(signal_num);
}

// This functions is used on a for_each to close socket on each Connection object of a vector
void	closeSocket(Connection objet)
{
	if (close(objet.getSocket()) == -1)
		PRINT_RED("Closing connection failed ")<<"on socket "<<objet.getSocket()<<" : "<<strerror(errno)<<std::endl;
	else
		std::cout<<"socket "<<objet.getSocket()<<" closed"<<std::endl;
	// Rajouter
	objet.closeRequestCGIPipe();
}
