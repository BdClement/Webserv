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
#include "ServerConfig.hpp"

// class Config;
class ServerConfig;

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
	// bool	interfaceAlreadyExist(std::vector<Config>::iterator const& toFind);
	bool	interfaceAlreadyExist(std::vector<ServerConfig>::iterator const& toFind);

	// PARSING CONFIG FILE
	// void	initTestConfig(char * arg);//fonction qui joue le role du parsing
	bool	initTestConfig(char * arg);
	int		findServerBlock(Connection & connection);
	int		findLocationBlock(ServerConfig & serverBlock, std::string & uri);

	// SERVER INITIALISATION

	void	initServer();
	// bool	initListenConnection(std::vector<Config>::iterator & it, Connection & new_connection);// Init struct sockaddr_in et epoll_event
	bool	initListenConnection(std::vector<ServerConfig>::iterator & it, Connection & new_connection);

	// LAUNCHING SERVER
	void	launchServer();
	void	acceptIncomingConnection(int const socket);
	// Epollin
	void	epollinEvent(struct epoll_event & events, int index);
	void	handlingEpollinEvent(Connection & connection);
	void	handlingCgiEpollin(Connection & connection, int &fd);
	// Epollout
	void	epolloutEvent(struct epoll_event & events, int index);
	void	handlingEpolloutEvent(Connection & connection);
	void	handlingCgiEpollout(Connection & connection, int &fd);
	// Epollhup
	void	epollhupEvent(struct epoll_event & events, int index);
	void	handlingCgiEpollhup(Connection & connection, int &fd);
	// Epollerr
	void	handlingEpollerrEvent(Connection & connection);
	// Event Monitoring
	void	addEpollout(Connection & connection);
	void	rmEpollout(Connection & connection);
	void	setPipeMonitoring(Connection & connection);

	// CLEANING SERVER
	void	closeAndRmConnection(Connection & connection);
	void	handlingKeepAlive();

	private:

	// std::vector<Config>			m_config;// Objets issus du Parsing du config file
	std::vector<ServerConfig>	m_config;
	std::vector<Connection>		m_listen_connection;// Sockets d'ecoutes
	std::vector<Connection>		m_http_connection;// Sockets de communication HTTP

	int							epoll_fd;
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
	// std::vector<Config>							* global_config;
	std::vector<ServerConfig>					* global_config;
	std::vector<Connection> 					* global_listen_connection;
	std::vector<Connection> 					* global_http_connection;
	int											* global_epoll_fd;
};

// Utils functions
unsigned long	convertAddr(std::string to_convert);
std::string 	convertAddrBack(unsigned long ip_addr);
void			signalHandler(int signal_num);
void			closeSocket(Connection objet);

