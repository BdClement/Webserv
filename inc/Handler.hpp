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
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <vector>
#include "algorithm"

#include "Connection.hpp"
#include "Config.hpp"


class Handler
{
	public:
	Handler();
	Handler(Handler const& asign);
	~Handler();

	Handler & operator=(Handler const& equal);

	// Predicat pour determiner si une connection est une connection d'ecoute
	struct CompareSocket{
	int	searchSocket;
	CompareSocket(int socket) : searchSocket(socket){}
	bool operator()(const Connection& obj) const{
		return obj.socket == searchSocket;
	}
	};

	// UTIILS
	int		recoverIndexConnection(int const socket) const;
	bool	isListenningConnection(int const socket);
	bool	interfaceAlreadyExist(std::vector<Config>::iterator const& toFind);

	// PARSING CONFIG FILE
	void	initTestConfig();//fonction qui joue le role du parsing

	// SERVER INITIALISATION

	void	initServer();
	void	initListenConnection(std::vector<Config>::iterator & it, Connection & new_connection);// Init struct sockaddr_in et epoll_event

	// LAUNCHING SERVER
	void	launchServer();
	void	acceptIncomingConnection(int const socket);
	void	handlingEpollinEvent(int const index);
	void	handlingEpolloutEvent(int const index);
	void	handlingEpollerrEvent(int const index);


	// CLEANING SERVER
	void	closeAndRmConnection(int const index);

	private:

	std::vector<Config>			m_config;// Objets issus du Parsing du config file
	std::vector<Connection>		m_listen_connection;// Sockets d'ecoutes
	std::vector<Connection>		m_http_connection;// Sockets de communication HTTP

	int							epoll_fd;

	// int	error_code; // ? Pour le retour de notre programme si erreur rencontrer
};

#endif
