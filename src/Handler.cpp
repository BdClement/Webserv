/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handler.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/16 12:55:53 by clbernar          #+#    #+#             */
/*   Updated: 2024/04/22 21:49:44 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Handler.hpp"
#include <unistd.h> // Pour sleep

Handler::Handler()
{
	std::cout<<"Creation de l'objet Handler"<<std::endl;
}

Handler::Handler(Handler const& asign)
{
	(void)asign;
	// Arguments a ajouter a l'objet au fur et a mesure
	// Cet objet ne sera de toute facon pas destine a etre copie
}

Handler::~Handler()
{
	std::cout<<"Destruction de l'objet Handler"<<std::endl;
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
void	Handler::init_server()
{
	// CREATE LISTEN_SOCKETS
	std::cout<<"Creations des sockets d'ecoutes specifiees lors de la configuration"<<std::endl;

	// Creation de structure representant des blocs server necessaire car nous aurons besoin
	// de ces structures pour utiliser des fonctions de gestion de socket par la suite telle que bind()
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
		struct epoll_event	events[20];// Taille a debattre
		int	num_event = epoll_wait(epoll_fd, events, 20, 5);
		if (num_event == -1)
		{
			std::cout<<"Wait failed"<<std::endl;
			//errno
			break;
		}
		if (num_event >= 0)
		{
			std::cout<<"num_event = "<<num_event<<std::endl;
			for (int i = 0; i < num_event; ++i)
			{
				// Logique de separtion sockets d'ecoutes / sockets de communiactation
				std::cout<<"Connexion entrante recue"<<std::endl;
				// Logique de socket d'ecoute
				if (events[i].data.fd == socket_fd)
				{
					struct sockaddr_in	client_addr;
					socklen_t client_len = sizeof(client_addr);
					int client_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &client_len);
					if (client_fd == -1)
						std::cout<<"Accept failed"<<std::endl;
					else
					{
						std::cout<<"Connexion entrante etablie sur la socket "<< client_fd<<std::endl;
						sleep(3);
						struct epoll_event	event_com;
						event_com.events = EPOLLIN | EPOLLOUT | EPOLLERR;
						event_com.data.fd = client_fd;
						if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event_com) == -1)
							std::cout<<" EPOLL_CTL_ADD failed (dans test): "<<strerror(errno)<<std::endl;
						else
							std::cout<<"Ajout de la socket de communication a epoll_fd"<<std::endl;
					}
				}
				else // Logique de socket de communication
				{
					std::cout<<"Donnees recu sur la socket de communication "<<events[i].data.fd<<std::endl;
					sleep(1);
					// Logique de lecture de la requete recue
				}
				// this->test(client_fd, epoll_fd, client_addr);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////
// Classe : socket d'ecoute, socket de cmmunication , request, response

// void	Handler::test(int	new_connexion, int ensemble, struct sockaddr_in new_connexion_info)
// {
// 	(void)new_connexion_info;
// 	// Declaration de la structure d'evenement lie a cette connexion entrante
// 	struct epoll_event	event;
// 	event.events = EPOLLIN | EPOLLOUT | EPOLLERR;
// 	event.data.fd = new_connexion;
// 	// Ajout de cette socket a l'ensemble de surveillance
// 	if (epoll_ctl(ensemble, EPOLL_CTL_ADD, new_connexion, &event) == -1)
// 		std::cout<<" EPOLL_CTL_ADD failed (dans test): "<<strerror(errno)<<std::endl;
// }
