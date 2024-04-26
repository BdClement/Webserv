/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handler.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/16 12:55:53 by clbernar          #+#    #+#             */
/*   Updated: 2024/04/26 19:00:23 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Handler.hpp"
#include <unistd.h> // Pour sleep

Handler::Handler()
{
	// std::cout<<"Handler constructor called \n"<<std::endl;
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

// Les ports en dessous de 1024 ne peuvent pas etre bind() car l'OS a une politique de securite
// concernant ses ports la qui sont consideres comme reserves ou bien connus.
// Cette logique est a faire de maniere dynamique autant de fois que le fichier de configuration le demande (cf Notion)
// -Parcours des objets renvoyes par le Parsing
// -Pour chaque bloc initialisation d'une socket d'ecoute (non bloquant fcntl a utiliser)
// -Gerer le cas ou un interface a deja une socket d'ecoute et attribuer la meme socket d'ecoute
// -Ajouter la socket d'ecoute a l'ensemble de surveillance
void	Handler::init_server()
{
	// CREATE LISTEN_SOCKETS
	std::cout<<"Creations des sockets d'ecoutes specifiees lors de la configuration"<<std::endl;

	// Creation de structure representant des blocs server necessaire car nous aurons besoin
	// de ces structures pour utiliser des fonctions de gestion de socket par la suite telle que bind()
	// Memset pour initialiser les structures , POURQUOI ??
	listen_connexion.sin_family = AF_INET;
	listen_connexion.sin_port = htons(1025);// Port specifie
	listen_connexion.sin_addr.s_addr = htonl(convert_addr("127.0.0.1"));// IP Specifie

	// Creation de la socket
	socket_fd = socket(listen_connexion.sin_family, SOCK_STREAM, 0);
	if (socket_fd == -1)
		std::cout<<"Socket creation failed"<<std::endl;
	else
		std::cout<<"Socket_fd = "<<socket_fd<<std::endl;

	// Liason de la socket avec les infromation de connexion de la strucutre
	if (bind(socket_fd, (struct sockaddr *)&listen_connexion , sizeof(listen_connexion)) == -1)
	{
		std::cout<<"Bind failed : "<<strerror(errno)<<std::endl;
	}
	else
		std::cout<<"Bind successful"<<std::endl;

	// Mise en ecoute de la socket d'ecoute
	if (listen(socket_fd, 20) == -1)// 20 connexion maximum en simultanne par defaut
		std::cout<<"Listening failed : "<<strerror(errno)<<std::endl;
}


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
void	Handler::launch_server()
{
	// Creation de la logique de surveillance des sockets
	int	epoll_fd = epoll_create1(0); // A checker

	// Creation de la structure d'evenements commune aux sockets
	// Cette structure sert a specifie a quels evenements la socket doit reagir ou non
	// EPOLLIN disponibilite pour la lecture
	// EPOLLOUT disponibilite pour l'ecriture
	// EPOLLERR pour les erreurs rencontrees
	// Elle peut eventuellement etre utilise sur plusieurs sockets qui ont le meme objectif
	struct epoll_event	event;
	event.events = EPOLLIN;
	event.data.fd = socket_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1)
		std::cout<<" EPOLL_CTL_ADD failed : "<<strerror(errno)<<std::endl;

	// Boucle d'evenement
	while (1)
	{
		std::cout<<"TEST 1"<<std::endl;
		struct epoll_event	events[20];// Taille a debattre
		int	num_event = epoll_wait(epoll_fd, events, 20, 5);
		if (num_event == -1)
		{
			std::cout<<"Wait failed"<<std::endl;
			//errno
			break;
		}
		if (num_event == 0)
		{
			std::cout<<"No events received"<<std::endl;
		}
		if (num_event > 0)
		{
			for (int i = 0; i < num_event; ++i)
			{
				// std::cout<<"TEST 1"<<std::endl;
				// Evenement detecte
				std::cout<<"EPOLLIN == "<< EPOLLIN<<std::endl;
				std::cout<<"EPOLLOUT == "<< EPOLLOUT<<std::endl;
				std::cout<<"EPOLLERR == "<< EPOLLERR<<std::endl;
				// std::cout<<"Event detecte ==  "<<events[i].events<<std::endl;
				if (events[i].events == EPOLLIN)
					std::cout<<"Event EPOLLIN sur la socket => "<<events[i].data.fd<<std::endl;
				else if (events[i].events == EPOLLOUT)
					std::cout<<"Event EPOLLOUT sur la socket => "<<events[i].data.fd<<std::endl;
				else if (events[i].events == EPOLLERR)
					std::cout<<"Event EPOLLERR sur la socket => "<<events[i].data.fd<<std::endl;
				else
					std::cout<<"Event autre detecte ==  "<<events[i].events<<" sur la socket => "<<events[i].data.fd<<std::endl;
				// Logique de separtion sockets d'ecoutes / sockets de communiactation
				sleep(1);
				// Logique de socket d'ecoute (non bloquant fcntl a utiliser)
				if (events[i].data.fd == socket_fd)
				{
					std::cout<<"Event recu sur la socket d'ecoute = "<<socket_fd<<std::endl;
					struct sockaddr_in	client_addr;
					socklen_t client_len = sizeof(client_addr);
					this->client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_len);
					// int client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_len);
					if (this->client_fd == -1)
						std::cout<<"Accept failed"<<std::endl;
					else
					{
						std::cout<<"Nouvelle Connexion etablie via la socket d'ecoute => "<< client_fd<<std::endl;
						sleep(3);
						this->client_event.events = EPOLLIN | EPOLLERR;
						this->client_event.data.fd = this->client_fd;
						// struct epoll_event	event_com;
						// event_com.events = EPOLLIN | EPOLLERR;
						// event_com.data.fd = client_fd;
						// if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, this->client_fd, &event_com) == -1)
						if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, this->client_fd, &(this->client_event)) == -1)
							std::cout<<" EPOLL_CTL_ADD failed (dans test): "<<strerror(errno)<<std::endl;
						else
							std::cout<<"Ajout de la nouvelle socket de communication a epoll_fd"<<std::endl;
					}
				}
				else // Logique de socket de communication
				{
					std::cout<<"Event recu sur la socket de communication "<<events[i].data.fd<<std::endl;
					sleep(1);
					if (events[i].events & EPOLLIN)
					{
						std::cout<<"Event EPOLLIN recu sur la socket de communication "<<events[i].data.fd<<std::endl;
						char buffer[10000];
						memset(buffer, '\0', sizeof buffer);
						int bytes_read = recv(events[i].data.fd, buffer, sizeof(buffer), 0);
						// Etre sur que le dernier byte est a \0 dans buffer
						if (bytes_read <= 0)
						{
							if (bytes_read == 0)
								std::cout<<"Client closed connection"<<std::endl;
							else
								std::cout<<"Error : "<<strerror(errno)<<std::endl;
						}
						else
						{
							std::cout<<"Le message recu est : \n"<<buffer<<std::endl;
							std::cout<<"\nLogique de traitement de la requete a integrer"<<std::endl;
							std::cout<<"Logique de generation de reponse HTTP a integrer"<<std::endl;
							// |= "ou" binaire avec assignation. Permet de combiner les bits deja presents au bit EPOLLOUT que l'on ajoute
							// event_com.events |= EPOLLOUT;
							this->client_event.events |= EPOLLOUT;
							// if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &event_com) == 1)
							if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, this->client_fd, &(this->client_event)) == 1)
								std::cout<<"Ajout de l'evenement EPOLLOUT a surveiller sur la socket "<<client_fd<<" failed"<<std::endl;
							else
								std::cout<<"Ajout de l'evenement EPOLLOUT a surveiller sur la socket "<<client_fd<<" success"<<std::endl;
						}
					}
					if (events[i].events & EPOLLOUT)
					{
						std::cout<<"Event EPOLLOUT recu sur la socket de communication "<<events[i].data.fd<<std::endl;
						std::cout<<"Envoi de la reponse sur la socket"<<std::endl;
						// &= ~ "et" binaire avec negation de EPOLLOUT et assignation
						// Cela permet de supprimer le bit EPOLLOUT en inversant sa valeur
						// event_com.events &= ~EPOLLOUT;
						this->client_event.events &= ~ EPOLLOUT;
						// if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &event_com) == 1)
						if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, this->client_fd, &(this->client_event)) == 1)
							std::cout<<"Retrait de l'evenement EPOLLOUT a surveiller sur la socket "<<client_fd<<" failed"<<std::endl;
						else
							std::cout<<"Retrait de l'evenement EPOLLOUT a surveiller sur la socket "<<client_fd<<" success"<<std::endl;
					}
					if (events[i].events & EPOLLERR)
					{
						std::cout<<"Event EPOLLERR recu sur la socket de communication "<<events[i].data.fd<<std::endl;
					}
					// if (events[i].events != EPOLLOUT)
					// {
					// 	// Logique de lecture de la requete recue
					// 	char buffer[10000];
					// 	memset(buffer, '\0', sizeof buffer);
					// 	// std::cout<<"Test avant recv"<<std::endl;
					// 	int bytes_read = recv(events[i].data.fd, buffer, sizeof(buffer), 0);
					// 	// std::cout<<"Test apres recv"<<std::endl;
					// 	// Etre sur que le dernier byte est a \0 dans buffer
					// 	if (bytes_read <= 0)
					// 	{
					// 		if (bytes_read == 0)
					// 			std::cout<<"Client closed connection"<<std::endl;
					// 		else
					// 			std::cout<<"Error : "<<strerror(errno)<<std::endl;
					// 	}
					// 	else
					// 	{
					// 		std::cout<<"Le message recu est : \n"<<buffer<<std::endl;
					// 	}
					// }
				}
				std::cout<<std::endl;
			}
			std::cout<<"\nTour de boucle principale\n"<<std::endl;
		}
	}
}


// Pas d'IP =  Ecoute toutes les adresses IP
// Pas de port = Port par defaut 8080
// Pas de server_name = (pas de reel impact ici)
// A tester : PAs de port declarer, pas d'IP declarer, pas de server_name
// Rien de declarer, juste un server name. juste un port , juste une IP
// Une IP et un port , IP et server name, port et server name
// Plusieurs interfaces similaires avec / sans server name
void	Handler::init_test_config()
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

bool	Handler::interface_already_exist(std::vector<Config>::iterator toFind)
{
	for (std::vector<Config>::iterator it = m_config.begin(); it != m_config.end(); ++it)
	{
		if (it == toFind)
			return false;
		if (it->listen_addr == toFind->listen_addr && it->listen_port == toFind->listen_port)
		{
			std::cout<<"Pas de socket cree pour l'interface => "<<toFind->listen_addr<<" : "<<toFind->listen_port<<std::endl;
			return true;
		}
	}
	return false;
}

// Mettre une condition en cas d'erreur de ne pas ajouter la connection au vecteur !
// Faire la gestion des evenements 
void	Handler::init_listen_socket()
{
	this->epoll_fd = epoll_create1(0);
	int i = 0;

	// Utiliser un FOR_EACH ?
	for (std::vector<Config>::iterator it = m_config.begin(); it != m_config.end(); ++it)
	{
		std::cout<<"\n\n\nTour de boucle "<<i++<<std::endl;
		if (!interface_already_exist(it))
		{
			Connection	new_connection;
			// SOCKET
			new_connection.socket = socket(AF_INET, SOCK_STREAM, 0);
			if (new_connection.socket == -1)
				std::cout<<"Socket creation failed"<<std::endl;
			else
				std::cout<<"Socket created = "<<new_connection.socket<<std::endl;
			// INTERFACE
			new_connection.interface.sin_family = AF_INET;
			new_connection.interface.sin_port = htons(it->listen_port);
			if (it->listen_addr == "0.0.0.0")
			{
				std::cout<<"C'est bien rentre mon boug"<<std::endl;
				new_connection.interface.sin_addr.s_addr = INADDR_ANY;
			}
			else
				new_connection.interface.sin_addr.s_addr = htonl(convert_addr(it->listen_addr));
			//EVENT
			//
			//BIND
			if (bind(new_connection.socket, (struct sockaddr *)&(new_connection.interface), sizeof(new_connection.interface)) == -1)
				std::cout<<"Bind failed : "<<strerror(errno)<<std::endl;
			else
				std::cout<<"Bind successful"<<std::endl;
			//LISTEN
			if (listen(new_connection.socket, 20) == -1)
				std::cout<<"Listening failed : "<<strerror(errno)<<std::endl;
			else
				std::cout<<"Listenning success"<<std::endl;
			//Ajout de la socket d'ecoute au vecteur
			m_listen_connection.push_back(new_connection);
			if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, new_connection.socket, &(new_connection.event)) == -1)
				std::cout<<" EPOLL_CTL_ADD failed : "<<strerror(errno)<<std::endl;
			else
				std::cout<<"Ajout de la socket d'ecoute a epoll"<<std::endl;
			std::cout<<"Creation de la socket et mise en ecoute => "<<new_connection.socket<<" ecoutant sur l'interface "<<it->listen_addr<<" : "<<it->listen_port<<std::endl;
		}
	}
	std::cout<<"\n\n\nNombre de socket d'ecoute cree = "<<this->m_listen_connection.size()<<std::endl;
}

/////////////////////////////////////////////////////////////////////
// Classe : socket d'ecoute, socket de cmmunication , request, response

