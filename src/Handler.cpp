/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handler.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/16 12:55:53 by clbernar          #+#    #+#             */
/*   Updated: 2024/04/17 17:54:44 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Handler.hpp"

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

void	Handler::init_server()
{
	std::cout<<"Fonction init_server appelee !"<<std::endl;
	// for (int  i = 0; i < 2; i++)
	// {
		// Creation de la structure representant la connexion
		listen_connexion[0].sin_family = AF_INET;
		listen_connexion[0].sin_port = htons(80);
		if (inet_pton(AF_INET, "127.0.0.1", &(listen_connexion[0].sin_addr)) == 0)
			std::cout<<"IP conversion failed"<<std::endl;
		std::cout<<"Retour de conversion = "<<listen_connexion[0].sin_addr.s_addr<<std::endl;
		// Creation de la socket
		socket_fd[0] = socket(listen_connexion[0].sin_family, SOCK_STREAM, 0);
		if (socket_fd[0] == -1)
		{
			std::cout<<"Socket creation failed"<<std::endl;
			// Checker errno ?
		}
		// Liason de la socket cree a l'adresse IP et au port adequat
		if (bind(socket_fd[0], (struct sockaddr *)&listen_connexion[0] , sizeof(listen_connexion[0])) == -1)
		{
			std::cout<<"Bind failed"<<std::endl;
			// Checker errno ?
		}
		else
			std::cout<<"Bind successful"<<std::endl;
		// Mise en ecoute des sockets d'ecoutes
		if (listen(socket_fd[0], 20) == -1)// 20 connexion maximum en simultanne par defaut
		{
			std::cout<<"Listening failed"<<std::endl;
			// Checker errno ?
		}
	// }
}

// Gerer le nombre max de fd a surveiller ??
void	Handler::launch_server()
{
	std::cout<<"Fonction launch_server appelee !"<<std::endl;
	// Creation de la logique de Surveillance grace a epoll()
	int	epoll_fd = epoll_create1(0);
	// Creation de la structure d'evenements commune aux socket
	struct epoll_event	event;
	event.events = EPOLLIN; // Ajouter EPOLLOUT EPOLLERR
	event.data.fd = socket_fd[0];
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd[0], &event) == -1)
	{
		std::cout<<" EPOLL_CTL_ADD failed"<<std::endl;
		// Checker errno ?
	}
	else
		std::cout<<"OK"<<std::endl;
	while (1)
	{
		std::cout<<"O1"<<std::endl;
		struct epoll_event	events[20];
		//
		std::cout<<"OK2"<<std::endl;
		int	num = epoll_wait(epoll_fd, events, 20, 5);
		std::cout<<"num = "<<num<<std::endl;
		if (num == -1)
		{
			std::cout<<"Wait failed"<<std::endl;
			//errno
		}
		// for (int i = 0; i < num; ++i)
		// {
		// 	if (events[i].data.fd == socket_fd[0])
		// 	{
		// 		std::cout<<"Connexion entrante recu"<<std::endl;
		// 		struct sockaddr_in	client_addr;
		// 		socklen_t client_len = sizeof(client_addr);
		// 		int client_fd = accept(socket_fd[0], (struct sockaddr *)&client_addr, &client_len);
		// 		if (client_fd == -1)
		// 			std::cout<<"Accept failed"<<std::endl;
		// 		else
		// 			std::cout<<"Connexion entrante etablie"<<std::endl;
		// 	}
		// }
	}
}

/////////////////////////////////////////////////////////////////////
// Pourquoi les ports en dessous 1024 ne peuvent pas etre bind()
// Comprende la gestion d'evenement a partir de ce modele.


void	Handler::test()
{
	// Creation de la structure representant la connexion
	struct sockaddr_in	connexion;
	connexion.sin_family = AF_INET;
	connexion.sin_port = htons(1025); // Changement avec 8000 et bin successful ??
	if (inet_pton(AF_INET, "127.0.0.1", &(connexion.sin_addr)) == 0)
		std::cout<<"IP conversion failed"<<std::endl;
	std::cout<<"IP convertie = "<<connexion.sin_addr.s_addr<<" /// port au format reseau = "<<connexion.sin_port<<std::endl;

	// Creation de la socket
	int socket_fd = socket(connexion.sin_family, SOCK_STREAM, 0);
	if (socket_fd == -1)
		std::cout<<"Socket creation failed"<<std::endl;
	else
		std::cout<<"Socket_fd = "<<socket_fd<<std::endl;

	// Liason de la socket avec les infromation de connexion de la strucutre
	if (bind(socket_fd, (struct sockaddr *)&connexion , sizeof(connexion)) == -1)
	{
		std::cout<<"Bind failed : "<<strerror(errno)<<std::endl;
	}
	else
		std::cout<<"Bind successful"<<std::endl;

	// Mise en ecoute de la socket d'ecoute
	if (listen(socket_fd, 20) == -1)// 20 connexion maximum en simultanne par defaut
		std::cout<<"Listening failed : "<<strerror(errno)<<std::endl;




	// Creation de la logique de surveillance des sockets
	int	epoll_fd = epoll_create1(0); // A checker

	// Creation de la structure d'evenements commune aux sockets
	struct epoll_event	event;
	event.events = EPOLLIN;
	event.data.fd = socket_fd;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1)
		std::cout<<" EPOLL_CTL_ADD failed : "<<strerror(errno)<<std::endl;

	// Boucle d'evenement
	while (1)
	{
		struct epoll_event	events[20];
		int	num = epoll_wait(epoll_fd, events, 20, 5);
		if (num != 0)
			std::cout<<"num = "<<num<<std::endl;
		if (num == -1)
		{
			std::cout<<"Wait failed"<<std::endl;
			//errno
		}
	}
}
