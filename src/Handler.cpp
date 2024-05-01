/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handler.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/16 12:55:53 by clbernar          #+#    #+#             */
/*   Updated: 2024/05/01 18:41:18 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Handler.hpp"
#include <unistd.h> // Pour sleep

struct clearFromHanlder global = {NULL, NULL, NULL, 0};

/***********************************************************************************************************
 *                                                                                                         *
 *                                           CANONICAL FORM CLASS                                          *
 *                                                                                                         *
 ***********************************************************************************************************/

Handler::Handler()
{
	extern clearFromHanlder globalSignal;
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
		std::cout<<socket<<" is a listenning socket"<<std::endl;
		return true;
	}
	else
	{
		std::cout<<socket<<" is not a listenning socket"<<std::endl;
		sleep(2);// A SUPPRIMER
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
			std::cout<<"Socket not created for interface => "<<toFind->listen_addr<<" : "<<toFind->listen_port<<". This interface already has a dedicated socket."<<std::endl;
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
	this->epoll_fd = epoll_create1(0);
	int i = 0;

	for (std::vector<Config>::iterator it = m_config.begin(); it != m_config.end(); ++it)
	{
		std::cout<<"\n\n\nTour de boucle "<<i++<<std::endl;
		if (!interfaceAlreadyExist(it))
		{
			Connection	new_connection;
			// SOCKET
			new_connection.socket = socket(AF_INET, SOCK_STREAM, 0);
			if (new_connection.socket == -1)
			{
				PRINT_RED("Socket creation failed :")<<strerror(errno)<<std::endl;
				continue;
			}
			else
				PRINT_GREEN("Socket created ")<<new_connection.socket<<std::endl;
			this->initListenConnection(it, new_connection);
			//BIND
			if (bind(new_connection.socket, (struct sockaddr *)&(new_connection.interface), sizeof(new_connection.interface)) == -1)
			{
				PRINT_RED("Bind failed")<<" socket "<<new_connection.socket<<" to interface "<<it->listen_addr<<":"<<it->listen_port<<" : "<<strerror(errno)<<std::endl;
				if (close(new_connection.socket) == -1)
					PRINT_RED("Closing socket failed : ")<<strerror(errno)<<std::endl;// a gerer ?
				continue;
			}
			else
				PRINT_GREEN("Bind successful")<<" socket "<<new_connection.socket<<" to interface "<<it->listen_addr<<":"<<it->listen_port<<std::endl;
			//LISTEN
			if (listen(new_connection.socket, 20) == -1)
			{
				PRINT_RED("Listening failed : ")<<strerror(errno)<<std::endl;
				continue;
			}
			else
				PRINT_GREEN("Listenning successful")<<std::endl;
			//Ajout de la socket d'ecoute au vecteur
			m_listen_connection.push_back(new_connection);
			if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, new_connection.socket, &(new_connection.event)) == -1)
				PRINT_RED("Adding socket to epoll failed : ")<<strerror(errno)<<std::endl;
			else
				std::cout<<"Adding socket to epoll"<<std::endl;
		}
	}
	std::cout<<"\n\n\nNombre de socket d'ecoute cree = "<<this->m_listen_connection.size()<<std::endl;
}

// This functions fills structures sockaddr_in and epoll_event of Connection object
void	Handler::initListenConnection(std::vector<Config>::iterator & it, Connection & new_connection)
{
	std::cout<<"New connection object initliazed [Interface and event]"<<std::endl;
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
					// On retrouve l'objet Connection associe a la socket qui recoit un evenement
					int	index = recoverIndexConnection(events[i].data.fd);
					if (index == -1)
					{
						PRINT_RED("Connection recovery failed")<<std::endl;
						continue;
					}
					// Logique de gestion des evenements
					if (events[i].events & EPOLLIN)
						handlingEpollinEvent(index);
					else if (events[i].events & EPOLLOUT)
						handlingEpolloutEvent(index);
					else if (events[i].events & EPOLLERR)
						handlingEpollerrEvent(index);
					else if (events[i].events & EPOLLHUP)
					{
						PRINT_RED("Connection closed by client on socket ")<<m_http_connection[index].socket<<std::endl;
						closeAndRmConnection(index);// A checker si c'est necessaire ici : Choix
					}
				}
			}
		}
		// Fonction qui check mes http_connections et supprime les keep_alive qui ont dure trop de temps
		if (num_event > MAX_EVENTS)
			std::cout<<"Warning : more events than MAX_EVENTS have been detected. It could affect server's performance"<<std::endl;
	}
}

// This function accept incoming connection, store the socket in a dedicated Connection object
// and add the socket to epoll
void	Handler::acceptIncomingConnection(int const socket)
{
	std::cout<<"Event recu sur la socket d'ecoute "<<socket<<std::endl;
	Connection new_connection;
	socklen_t	client_len = sizeof(new_connection.interface);
	new_connection.socket = accept(socket, (struct sockaddr *)&(new_connection.interface), &client_len);
	if (new_connection.socket == -1)
	{
		PRINT_RED("Accept connexion failed : ")<<strerror(errno)<<std::endl;
		return;
	}
	else
	{
		PRINT_GREEN("New connection established : ")<<"socket "<<new_connection.socket<<std::endl;
		new_connection.event.events = EPOLLIN | EPOLLERR | EPOLLHUP;
		new_connection.event.data.fd = new_connection.socket;
		if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, new_connection.socket, &(new_connection.event)) == -1)
			PRINT_RED("Adding new incoming connection to epoll failed : ")<<strerror(errno)<<std::endl;
		else
			PRINT_GREEN("Addinn new incoming connection to epoll")<<std::endl;
		m_http_connection.push_back(new_connection);
		sleep(3);
	}
}

// Attention a la gestion des requetes fragmentees
// Clean conneciton en cas d'erreur de lecture de la requete
// This function reads request and treat it with it Connection object
// It creates an http Response and make EPOLLOUT event on the socket as the response is ready
void	Handler::handlingEpollinEvent(int const index)
{
	std::cout<<"Event EPOLLIN detected on socket "<<m_http_connection[index].socket<<std::endl;
	//Logique de lecture de la donnee recue (Requete)
	char	buffer[10000];
	memset(buffer, '\0', sizeof(buffer));
	int bytes_read = recv(m_http_connection[index].socket, buffer, sizeof(buffer), 0);
	// S'assurer que le dernier element de buffer est bien \0
	if (bytes_read <= 0)
	{
		if (bytes_read == 0)
			std::cout<<"Client closed connection"<<std::endl;
		else
			std::cout<<"Error : "<<strerror(errno)<<" [Attention WARNING : pas le droit d'utiliser errno ici == A MODIFIER]"<<std::endl;
		closeAndRmConnection(index);// A checker peut etre en focntion de l'erreur de recv ???
	}
	else
	{
		std::cout<<"Le message recu est :\n"<<buffer<<std::endl;
		std::cout<<"Logique de traitement de requete ici"<<std::endl;
		std::cout<<"Logique de generation de reponse HTTP a integrer ici"<<std::endl;
		// |= "ou" binaire avec assignation. Permet de combiner les bits deja presents au bit EPOLLOUT que l'on ajoute
		m_http_connection[index].event.events |= EPOLLOUT;
		// On declare qu'on est pret a repondre sur la socket concernee (A voir avec les requetes fragmentees)
		if (epoll_ctl(this->epoll_fd, EPOLL_CTL_MOD, m_http_connection[index].socket, &(m_http_connection[index].event)) == -1)
		{
			PRINT_RED("Adding EPOLLOUT to watched event failed :")<<" socket "<<m_http_connection[index].socket<<std::endl;
			closeAndRmConnection(index);// A checker si c'est necessaire ici : Choix
		}
		else
			PRINT_GREEN("Adding EPOLLOUT to watched event")<<" socket "<<m_http_connection[index].socket<<std::endl;
	}
}

// Attention a la gestion des requetes fragmentees
// Clean connection en cas d'erreur d'envoi de la requete
// This Function sends the response on the socket and remove EPOLLOUT event on this socket as the reponse has been sent
void	Handler::handlingEpolloutEvent(int const index)
{
	std::cout<<"Event EPOLLOUT detected on socket "<<m_http_connection[index].socket<<std::endl;
	std::cout<<"Envoi de la reponse generee avec send surement"<<std::endl;
	// &= ~ "et" binaire avec negation de EPOLLOUT et assignation
	// Cela permet de supprimer le bit EPOLLOUT en inversant sa valeur
	m_http_connection[index].event.events &= ~EPOLLOUT;
	//On declare qu'on ne surveille plus EPOLLOUT puisqu'on a envoye notre reponse
	if (epoll_ctl(this->epoll_fd, EPOLL_CTL_MOD, m_http_connection[index].socket, &(m_http_connection[index].event)) == -1)
	{
		PRINT_RED("Removing EPOLLOUT to watched event failed :")<<" socket "<<m_http_connection[index].socket<<std::endl;
		closeAndRmConnection(index);// A checker si c'est necessaire ici : Choix
	}
	else
		PRINT_GREEN("Removing EPOLLOUT to watched event")<<" socket "<<m_http_connection[index].socket<<std::endl;
	// Si !keep alive dans la requete  => close connection
}

// Clean connection en cas d'event EPOLLERR detecte : En fonction de l'erreur ou tout le temps ?
void	Handler::handlingEpollerrEvent(int const index)
{
	std::cout<<"Event EPOLLERR detected on socket "<<m_http_connection[index].socket<<" : "<<strerror(errno)<<std::endl;
	// Gestion d'erreur
	closeAndRmConnection(index);// A checker si c'est necessaire ici : Choix
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
void	Handler::closeAndRmConnection(int const index)
{
	if (close(m_http_connection[index].socket) == -1)
	{
		PRINT_RED("Closing connection failed ")<<"on socket "<<m_http_connection[index].socket<<" : "<<strerror(errno)<<std::endl;
	}
	std::cout<<"Connection socket "<<m_http_connection[index].socket<<" closed"<<std::endl;
	std::swap(m_http_connection[index], m_http_connection.back());
	m_http_connection.pop_back();
	// std::cout<<"TEST size de vecteur m_http = "<<m_http_connection.size()<<std::endl;
}

/***********************************************************************************************************
 *                                                                                                         *
 *                                                 BROUILLON                                               *
 *                                                                                                         *
 ***********************************************************************************************************/

// Classe : socket d'ecoute, socket de cmmunication , request, response

// A Faire :
//			-Penser a l'utilisation d'exceptions
//			-Demarrer la gestion des requetes
