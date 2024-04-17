/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handler.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/16 12:46:54 by clbernar          #+#    #+#             */
/*   Updated: 2024/04/17 11:34:06 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// Cette classe sera l'objet principal qui represente notre programme de serveur web

#ifndef __HANDLER__HPP
#define __HANDLER__HPP

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>

#include "webserv.hpp"


class Handler
{
	public:
	Handler();
	Handler(Handler const& asign);
	~Handler();

	Handler & operator=(Handler const& equal);

	// ParsingConfig qui retourne des objets contenant l'ensemble des informations (a definir)
	// Avec sous objet bloc Location (a definir aussi)

	// A partir du parsing preparer les connexions c'est a dire creer des structs sockaddr_in
	// a stocker dans les objets cree lors du parsing
	// ajouter peut etre une gestion du cas ou y'a pas d'IP seulement un nom de domaine avec getaddrinfo()
	//bind les sockets d'ecoutes avec l'adresse IP et listen
	void	init_server();
	// Faire la boucle principale de surveillance des sockets grace
	void	launch_server();
	void	test();
	private:

	//Stockage des objets retourne par Paring (conteneur ?)
	struct	sockaddr_in	listen_connexion[2]; // Directement stocke ca dans les objets retourne par le Parsing ?
	int					socket_fd[2];
	// int	error_code; // ? Pour le retour de notre programme si erreur rencontrer
};

#endif
