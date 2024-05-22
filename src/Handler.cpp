/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handler.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/16 12:55:53 by clbernar          #+#    #+#             */
/*   Updated: 2024/05/22 20:24:30 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Handler.hpp"
#include "Config.hpp"

struct clearFromHanlder global = {NULL, NULL, NULL, 0,};

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
	// Arguments a ajouter a l'objet au fur et a mesure
	// Cet objet ne sera de toute facon pas destine a etre copie
}

Handler::~Handler()
{
	// std::cout<<"Handler destructor called\n"<<std::endl;
}

Handler& Handler::operator=(Handler const & equal)
{
	if (this != &equal)
	{
		// Pareil que le constructeur de copie
	}
	return *this;
}

/***********************************************************************************************************
 *                                                                                                         *
 *                                                 UTILS                                                   *
 *                                                                                                         *
 ***********************************************************************************************************/

// UTILS This function returns index of Connection object representing a socket
int		Handler::recoverIndexConnection(int const socket) const
{
	for (int i = 0; i < (int)m_http_connection.size(); ++i)
	{
		if (m_http_connection[i].socket == socket)
			return i;
	}
	return -1;
}

// UTILS This function checks if a socket is a listening socket
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
		// sleep(2);// A SUPPRIMER
		return false;
	}
}

// UTILS This function checks if a socket has already been created for an interface
bool	Handler::interfaceAlreadyExist(std::vector<Config>::iterator const& toFind)
{
	for (std::vector<Config>::iterator it = m_config.begin(); it != m_config.end(); ++it)
	{
		if (it == toFind)
			return false;
		if (it->listen_addr == toFind->listen_addr && it->listen_port == toFind->listen_port)
		{
			PRINT_RED("Socket not created")<<" interface ["<<toFind->listen_addr<<":"<<toFind->listen_port<<"]. This interface already has a dedicated socket."<<std::endl;
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

 // PARTIE BASTIEN
// Pas d'IP =  Ecoute toutes les adresses IP
// Pas de port = Port par defaut 8080
// Pas de server_name = (pas de reel impact ici)
// A tester : PAs de port declarer, pas d'IP declarer, pas de server_name
// Rien de declarer, juste un server name. juste un port , juste une IP
// Une IP et un port , IP et server name, port et server name
// Plusieurs interfaces similaires avec / sans server name
void	Handler::initTestConfig()// PARTIE PARSING
{
	Config	test1("127.0.0.1", 8080, "");
	Config	test2("0.0.0.0", 8080, "");
	Config	test3("127.0.0.1", 1025, "");
	Config	test4("0.0.0.0", 1028, "");
	Config	test5("127.0.0.1", 8080, "test.42.fr");
	Config	test6("0.0.0.0", 8080, "");
	m_config.push_back(test1);
	m_config.push_back(test2);
	m_config.push_back(test3);
	m_config.push_back(test4);
	m_config.push_back(test5);
	m_config.push_back(test6);
	// std::cout<<"Test initialisation des objets Config"<<std::endl;
	// for (int i = 0; i < 6; ++i)
	// 	std::cout<<m_config[i].listen_addr<<" "<<m_config[i].listen_port<<" "<<m_config[i].server_name<<std::endl;
}

/***********************************************************************************************************
 *                                                                                                         *
 *                                           SERVER INITIALISATION                                         *
 *                                                                                                         *
 ***********************************************************************************************************/

 // Les ports en dessous de 1024 ne peuvent pas etre bind() car l'OS a une politique de securite
// concernant ses ports la qui sont consideres comme reserves ou bien connus.
// Cette logique est a faire de maniere dynamique autant de fois que le fichier de configuration le demande (cf Notion)
// -Parcours des objets renvoyes par le Parsing
// -Pour chaque bloc initialisation d'une socket d'ecoute (non bloquant fcntl a utiliser)
// -Gerer le cas ou un interface a deja une socket d'ecoute et attribuer la meme socket d'ecoute
// -Ajouter la socket d'ecoute a l'ensemble de surveillance
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
	for (std::vector<Config>::iterator it = m_config.begin(); it != m_config.end(); ++it)
	{
	            //    ************************************************************
		std::cout<<"\n\n==== Config Block "<<(i++ + 1)<<" ===="<<std::endl;
		if (!interfaceAlreadyExist(it))
		{
			Connection	new_connection;
			if (!this->initListenConnection(it, new_connection))
				continue;
			//BIND
			std::cout<<"--- Binding socket to interface ---"<<std::endl;
			if (bind(new_connection.socket, (struct sockaddr *)&(new_connection.interface), sizeof(new_connection.interface)) == -1)
			{
				PRINT_RED("\tFail")<<" : Socket "<<new_connection.socket<<" to interface "<<it->listen_addr<<":"<<it->listen_port<<" : "<<strerror(errno)<<std::endl;
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
				PRINT_GREEN("\tSuccess")<<" : socket "<<new_connection.socket<<" to interface ["<<it->listen_addr<<":"<<it->listen_port<<"]"<<std::endl;
		}
	}
	PRINT_GREEN("Initialisation finished")<<". "<<this->m_listen_connection.size()<<" listenning socket(s) created in total.\n"<<std::endl;
}

// This functions fills structures sockaddr_in and epoll_event of Connection object
bool	Handler::initListenConnection(std::vector<Config>::iterator & it, Connection & new_connection)
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
	// if (fcntl(new_connection.socket, F_SETFL, O_NONBLOCK) < 0)
	// {
	// 	PRINT_RED("Error O_NONBLOCK failed on socket")<<" : Listenning socket "<<new_connection.socket<<" closed"<<std::endl;
	// 	close(new_connection.socket);
	// 	return false;
	// }
	std::cout<<"--- Connection object initialization ---"<<std::endl;
	PRINT_GREEN("\tSuccess [Interface and event]")<<std::endl;
	// INTERFACE
	new_connection.interface.sin_family = AF_INET;
	new_connection.interface.sin_port = htons(it->listen_port);
	if (it->listen_addr == "0.0.0.0")
	{
		std::cout<<"C'est bien rentre mon boug"<<std::endl;
		new_connection.interface.sin_addr.s_addr = INADDR_ANY;
	}
	else
		new_connection.interface.sin_addr.s_addr = htonl(convertAddr(it->listen_addr));
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

// Gerer le nombre max de fd a surveiller ??
// Double connexion via google-chrome pour une connexion entrante ??
// Close proprement les connexions car parfois erreur de bind addresse deja utilisee
// a cause de CTRL+C pour stopper le serveur
// Reponse : Les connections TCP ne sont pas directement free (meme avce un close), la socket
// devient en mode TIME_WAIT pour donner du temps a la socket de ferme proprement
// apres un certain temps l'OS libere l'adresse (voir les solutions pour eviter ca)
// -Boucle principale a mettre en place
// -2 cas principaux :
// 		-Logique d'ecoute : acceptation des connexions et creation d'un objet dedie a une socket de communication
// 		-Logique de communication : Gestion d'evenements EPOLLIN | EPOLLOUT | EPOLLERR
// Mettre en place une logique de fermeture des Connections (Temps d'attente trop long, detection de la fermeture de la connection)
// This function is the main monitoring loop to detect event and handle them
void	Handler::launchServer()
{
	// TEST SIGNAUX AVEC FORK (Pour CGI)
	// pid_t	pid = fork();
	// if (pid == -1)
	// 	PRINT_RED("Fork failed")<<std::endl;
	// if (pid == 0)
	// {
	// 	// isMainProcess = false;
	// 	sleep(100);
	// }
	std::cout<<"\n*****************       SERVER LAUNCH      *****************\n"<<std::endl;
	while (1)
	{
		struct epoll_event	events[MAX_EVENTS];// Taille a debattre & memset ?
		int	num_event = epoll_wait(epoll_fd, events, MAX_EVENTS, 5);
		if (num_event == -1)
		{
			PRINT_RED("Waiting event failed : ")<<strerror(errno)<<std::endl;
			break;
		}
		// else if (num_event == 0)
		// {
		// 	std::cout<<"No events received"<<std::endl;
		// }
		else
		{
			for (int i = 0; i < num_event; ++i)
			{
				// LOGIQUE D'ACCEPTATION DE CONNEXION ENTRANTE
				if (isListenningConnection(events[i].data.fd))
					acceptIncomingConnection(events[i].data.fd);
				else// LOGIQUE DE GESTION DE SOCKET DE COMMUNICATION
				{
					std::cout<<"\n=== Handling event... ===\n"<<std::endl;
					// On retrouve l'objet Connection associe a la socket qui recoit un evenement
					int	index = recoverIndexConnection(events[i].data.fd);
					if (index == -1)
					{
						PRINT_RED("\tConnection recovery failed")<<std::endl;
						continue;
					}
					// Logique de gestion des evenements
					if (events[i].events & EPOLLIN)
						handlingEpollinEvent(m_http_connection[index]);
						// handlingEpollinEvent(index);
					else if (events[i].events & EPOLLOUT)
						handlingEpolloutEvent(m_http_connection[index]);
					else if (events[i].events & EPOLLERR)
						handlingEpollerrEvent(m_http_connection[index]);
					else if (events[i].events & EPOLLHUP)
					{
						PRINT_RED("Connection closed by client on socket ")<<m_http_connection[index].socket<<std::endl;
						closeAndRmConnection(m_http_connection[index]);
					}
				}
			}
		}
		handlingKeepAlive();
		// Fonction qui check mes http_connections et supprime les keep_alive qui ont dure trop de temps
		if (num_event > MAX_EVENTS)
			std::cout<<"Warning : more events than MAX_EVENTS have been detected. It could affect server's performance"<<std::endl;
	}
}

// This function accept incoming connection, store the socket in a dedicated Connection object
// and add the socket to epoll
void	Handler::acceptIncomingConnection(int const socket)
{
	std::cout<<"\n=== Accept connection from listenning socket... ===\n"<<std::endl;
	std::cout<<"[ Event detected on listenning socket "<<socket<<"\n";
	Connection new_connection;
	socklen_t	client_len = sizeof(new_connection.interface);
	new_connection.socket = accept(socket, (struct sockaddr *)&(new_connection.interface), &client_len);
	std::cout<<"  New connection established : ";
	if (new_connection.socket == -1)
	{
		PRINT_RED("Failed ")<<": "<<strerror(errno)<<std::endl;
		return;
	}
	else
	{
		PRINT_GREEN("Success")<<" : socket "<<new_connection.socket<<"\n";
		// if (fcntl(new_connection.socket, F_SETFL, O_NONBLOCK) < 0)
		// {
		// 	PRINT_RED("Error O_NONBLOCK failed on socket")<<" : Connection established closed"<<std::endl;
		// 	close(new_connection.socket);
		// 	return ;
		// }
		// else
		// 	PRINT_GREEN("SOCKET DEVENUE NON BLOQUANTE")<<std::endl;
		new_connection.event.events = EPOLLIN | EPOLLERR | EPOLLHUP;
		new_connection.event.data.fd = new_connection.socket;
		std::cout<<"  Adding new socket to epoll : ";
		if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, new_connection.socket, &(new_connection.event)) == -1)
			PRINT_RED("Failed ")<<": "<<strerror(errno)<<" ]"<<std::endl;
		else
			PRINT_GREEN("Success")<<" ]"<<std::endl;
		m_http_connection.push_back(new_connection);
	}
}

// Attention a la gestion des requetes fragmentees
// Clean conneciton en cas d'erreur de lecture de la requete
// This function reads request and treat it with it Connection object
// It creates an http Response and make EPOLLOUT event on the socket as the response is ready
void	Handler::handlingEpollinEvent(Connection & connection)
{
	// SET TIME KEEP-ALIVE
	connection.last_active_time = time(NULL);
	// READ
	std::cout<<"[ Event EPOLLIN detected on socket "<<connection.socket<<std::endl;
	unsigned char	buffer[BUFFER_SIZE];
	memset(buffer, '\0', sizeof(buffer));
	int bytes_read = recv(connection.socket, buffer, sizeof(buffer), 0);
	// S'assurer que le dernier element de buffer est bien \0
	// if (bytes_read == EWOULDBLOCK || bytes_read == EAGAIN)
	// {
	// 	PRINT_RED("\tERREUR DETECTEE GRACE AU CARACTERE NON BLOQUANT DE LA SOCKET")<<std::endl;
	// 	return;
	// }
	if (bytes_read <= 0)
	{
		if (bytes_read == 0)
			std::cout<<"  Client closed connection]"<<std::endl;
		else
			std::cout<<"  Error : "<<strerror(errno)<<" [Attention WARNING : pas le droit d'utiliser errno ici == A MODIFIER]"<<std::endl;// A MODIFIER
		closeAndRmConnection(connection);// A checker peut etre en focntion de l'erreur de recv ???
	}
	else
	{
		//PARSE REQUEST
		connection.request.m_read.insert(connection.request.m_read.end(), buffer, buffer + bytes_read);
		std::cout<<"\nMessage received :\n["<<std::endl;
		for (std::vector<unsigned char>::iterator it = connection.request.m_read.begin(); it != connection.request.m_read.end(); ++it)// AFFICHAGE DE TEST
			std::cout<<*it;
		std::cout<<"]"<<std::endl;
		connection.request.parseRequest();
		if (connection.request.m_requestIsComplete)
		{
			std::cout<<"[ Logique de traitement de requete ]"<<std::endl;
			connection.request.processRequest(m_config, connection.response);
			//GENERATE RESPONSE => Objet Connection Concerne, Vector de Config
			std::cout<<"[ Logique de generation de reponse HTTP ]"<<std::endl;
			connection.response.generateResponse(connection.request);
			// sleep(5);
			// PRINT_GREEN("\n\n  FIN DU SLEEP DE TRAITEMENT DE REQUETE\n")<<std::endl;
			// DES QUE LA REPONSE EST PRETE
			// |= "ou" binaire avec assignation. Permet de combiner les bits deja presents au bit EPOLLOUT que l'on ajoute
			connection.event.events |= EPOLLOUT;
			// On declare qu'on est pret a repondre sur la socket concernee (A voir avec les requetes fragmentees)
			if (epoll_ctl(this->epoll_fd, EPOLL_CTL_MOD, connection.socket, &(connection.event)) == -1)
				PRINT_RED("  Adding EPOLLOUT to watched event failed :")<<" socket "<<connection.socket<<std::endl;
			else
				PRINT_GREEN("  Adding EPOLLOUT to watched event")<<" socket "<<connection.socket<<std::endl;
		}
	}
}

// Attention a la gestion des requetes fragmentees
// Clean connection en cas d'erreur d'envoi de la requete
// This Function sends the response on the socket and remove EPOLLOUT event on this socket as the reponse has been sent
void	Handler::handlingEpolloutEvent(Connection & connection)
{
	std::cout<<"  Event EPOLLOUT detected on socket "<<connection.socket<<std::endl;
	const unsigned char* rep = &(connection.response.m_response[0]);
	PRINT_GREEN("Contenu de ce qui a ete envoye = ")<<std::endl;
	for (std::vector<unsigned char>::iterator it = connection.response.m_response.begin(); it != connection.response.m_response.end(); ++it)
		std::cout<<*it;
	std::cout<<std::endl;
	if (send(connection.socket, rep, connection.response.m_response.size(), 0) == -1)
		PRINT_RED("  ERROR couldnt send response")<<std::endl;
	else
		PRINT_GREEN(" Response sent")<<std::endl;
	// &= ~ "et" binaire avec negation de EPOLLOUT et assignation
	// Cela permet de supprimer le bit EPOLLOUT en inversant sa valeur
	connection.event.events &= ~EPOLLOUT;
	//On declare qu'on ne surveille plus EPOLLOUT puisqu'on a envoye notre reponse
	if (epoll_ctl(this->epoll_fd, EPOLL_CTL_MOD, connection.socket, &(connection.event)) == -1)
	{
		PRINT_RED("  Removing EPOLLOUT to watched event failed :")<<" socket "<<connection.socket<<"]"<<std::endl;
		closeAndRmConnection(connection);//Fermeture car surement boucle infinie de EPOLLOUT
	}
	else
		PRINT_GREEN("  Removing EPOLLOUT to watched event")<<" socket "<<connection.socket<<"]"<<std::endl;
	// Si !keep alive dans la requete  => close connection
	if (!connection.request.isKeepAlive())
		closeAndRmConnection(connection);
	else
	{
		connection.response.clear();
		connection.request.clear();
	}
}

// Clean connection en cas d'event EPOLLERR detecte : En fonction de l'erreur ou tout le temps ?
void	Handler::handlingEpollerrEvent(Connection & connection)
{
	std::cout<<"\tEvent EPOLLERR detected on socket "<<connection.socket<<" : "<<strerror(errno)<<std::endl;
	// Gestion d'erreur
	closeAndRmConnection(connection);// A checker si c'est necessaire ici : Choix
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
	for (std::vector<Connection>::iterator it = m_http_connection.begin(); it != m_http_connection.end();)
	{
		if (difftime(time(NULL), it->last_active_time) > TIMEOUT)
		{
			// std::cout<<difftime(time(NULL), it->last_active_time)<<" > "<<TIMEOUT<<std::endl;
			std::vector<Connection>::iterator next = it;
			++next;
			if (close(it->socket) == -1)
				PRINT_RED("\tClosing connection failed ")<<"on socket "<<it->socket<<" : "<<strerror(errno)<<std::endl;
			else
				PRINT_GREEN("Connection socket closed : ")<<it->socket<<" | Kept alive 30 sec"<<std::endl;
			it = m_http_connection.erase(it);
			// PRINT_RED("JE suis sense retirer la connection qui n'a pas ete utilise depuis plus de 30 sec")<<std::endl;
		}
		else
			++it;
	}
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
	//VECTOR CONFIG
	std::vector<Config>().swap(*global.global_config);
	//VECTOR LISTENNING SOCKET
	std::cout<<"Cleaning listenning sockets"<<std::endl;
	std::for_each(global.global_listen_connection->begin(), global.global_listen_connection->end(), &closeSocket);
	std::vector<Connection>().swap(*global.global_listen_connection);
	//VECTOR HTTP SOCKET
	std::cout<<"Cleaning http communication sockets\n"<<std::endl;
	std::for_each(global.global_http_connection->begin(), global.global_http_connection->end(), &closeSocket);
	std::vector<Connection>().swap(*global.global_http_connection);
	//VECTOR VECTOR READING FROM SOCKET A garder en tete si leak ?
	// EPOLL
	if (close(*(global.global_epoll_fd)) == -1)
		PRINT_RED("Closing epoll_fd failed ")<<strerror(errno)<<std::endl;
	else
		std::cout<<"Epoll_fd closed"<<std::endl;
	exit(signal_num);
}

// This functions is used on a for_each to close socket on each Connection object of a vector
void	closeSocket(Connection objet)
{
	if (close(objet.getSocket()) == -1)
		PRINT_RED("Closing connection failed ")<<"on socket "<<objet.getSocket()<<" : "<<strerror(errno)<<std::endl;
	else
		std::cout<<"socket "<<objet.getSocket()<<" closed"<<std::endl;
}
