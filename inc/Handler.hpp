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

#pragma once

#include "webserv.hpp"
#include "Connection.hpp"

class Config;

class Handler
{
	public:
	Handler();
	Handler(Handler const& asign);
	~Handler();

	Handler & operator=(Handler const& equal);


	// UTIILS
	int		recoverIndexConnection(int const socket) const;
	bool	isListenningConnection(int const socket);
	bool	interfaceAlreadyExist(std::vector<Config>::iterator const& toFind);

	// PARSING CONFIG FILE
	void	initTestConfig();//fonction qui joue le role du parsing

	// SERVER INITIALISATION

	void	initServer();
	bool	initListenConnection(std::vector<Config>::iterator & it, Connection & new_connection);// Init struct sockaddr_in et epoll_event

	// LAUNCHING SERVER
	void	launchServer();
	void	acceptIncomingConnection(int const socket);
	void	handlingEpollinEvent(Connection & connection);
	void	addEpollout(Connection & connection);
	void	rmEpollout(Connection & connection);
	void	handlingEpolloutEvent(Connection & connection);
	void	handlingEpollerrEvent(Connection & connection);

	// CLEANING SERVER
	void	closeAndRmConnection(Connection & connection);
	void	handlingKeepAlive();

	private:

	std::vector<Config>			m_config;// Objets issus du Parsing du config file
	std::vector<Connection>		m_listen_connection;// Sockets d'ecoutes
	std::vector<Connection>		m_http_connection;// Sockets de communication HTTP

	int							epoll_fd;

	// int	error_code; // ? Pour le retour de notre programme si erreur rencontrer
};

// Predicat to determine if a socket is a listenning socket
struct CompareSocket{
int	searchSocket;
CompareSocket(int socket) : searchSocket(socket){}
bool operator()(const Connection& obj) const{
	return obj.socket == searchSocket;
}
};

// Global structure to exit proprely
struct clearFromHanlder{
	std::vector<Config>							* global_config;
	std::vector<Connection> 					* global_listen_connection;
	std::vector<Connection> 					* global_http_connection;
	int											* global_epoll_fd;
	// std::vector<std::vector<unsigned char>* >	*global_request_read;
	// std::vector<std::vector<unsigned char> >	global_request_read;
};

// Utils functions
unsigned long	convertAddr(std::string to_convert);
void			signalHandler(int signal_num);
void			closeSocket(Connection objet);

