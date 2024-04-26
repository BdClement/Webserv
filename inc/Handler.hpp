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
// #include <netinet/in.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <vector>

#include "Connection.hpp"
#include "Config.hpp"
// #include "webserv.hpp"


class Handler
{
	public:
	Handler();
	Handler(Handler const& asign);
	~Handler();

	Handler & operator=(Handler const& equal);



	void	init_server();
	// Faire la boucle principale de surveillance des sockets grace
	void	launch_server();
	// void	test(int	new_connexion, int ensemble, struct sockaddr_in new_connexion_info);

	// fonction qui joue le role du parsing
	void	init_test_config();
	void	init_listen_socket();
	bool	interface_already_exist(std::vector<Config>::iterator toFind);

	private:

	// Retour du parsing du fichier de config
	std::vector<Config>	m_config;
	// Sockets d'ecoutes
	std::vector<Connection>	m_listen_connection;
	// Sockets de communication HTTP
	std::vector<Connection>	m_http_connection;

	struct sockaddr_in	listen_connexion;
	int					socket_fd;
	int					client_fd;
	struct epoll_event	client_event;
	int					epoll_fd;

	// int	error_code; // ? Pour le retour de notre programme si erreur rencontrer
};

#endif
