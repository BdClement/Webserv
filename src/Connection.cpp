/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/25 15:38:08 by clbernar          #+#    #+#             */
/*   Updated: 2024/07/06 15:29:30 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"
#include "Handler.hpp"

Connection::Connection() : last_active_time(time(NULL))
{
	// std::cout<<"Connection constructor called"<<std::endl;
	// init interface sockaddr_in
	memset(&this->interface, 0, sizeof this->interface);
	// init event epoll_event
	memset(&this->event, 0, sizeof this->event);
}

Connection::Connection(Connection const& asign)
{
	// std::cout<<"Connection copy constructor called"<<std::endl;
	socket = asign.socket;
	interface = asign.interface;
	event = asign.event;
	request = asign.request;
	response = asign.response;
	last_active_time = asign.last_active_time;
	addr = asign.addr;
	port = asign.port;
}

Connection::~Connection()
{
	// extern clearFromHanlder global;
	// std::cout<<"Connection destructor called"<<std::endl;
	// std::vector<std::vector<unsigned char>* >::iterator it = std::find(global.global_request_read.begin(), global.global_request_read.end(), &(this->request.m_read));
	// if (it != global.global_request_read.end())
	// 	global.global_request_read.erase(it);
}

Connection& Connection::operator=(Connection const & equal)
{
	// std::cout<<"Connection operator called"<<std::endl;
	if (this != &equal)
	{
		this->socket = equal.socket;
		this->interface = equal.interface;
		this->event = equal.event;
		this->request= equal.request;
		this->response = equal.response;
		this->last_active_time = equal.last_active_time;
		this->addr = equal.addr;
		this->port = equal.port;
	}
	return *this;
}

int	Connection::getSocket() const
{
	return this->socket;
}

// This function handle killing child process and close pipe in CGI case
void	Connection::closeRequestCGIPipe()
{
	if (this->request.m_cgi)
	{
		if (this->request.m_pipe.m_pid != 0)
		{
			PRINT_RED("Kill du child process")<<std::endl;
			kill(this->request.m_pipe.m_pid, SIGINT);
			// int status;
			// waitpid(this->request.m_pipe.m_pid, &status, 0);
		}
		this->request.m_pipe.clear();
	}
}

bool	Connection::setAddrPort()
{
	struct sockaddr_in addr_tmp;
	socklen_t	len_addr_tmp = sizeof(addr);
	if (getsockname(this->socket, (struct sockaddr*)&addr_tmp, &len_addr_tmp) == -1)
	{
		close(this->socket);
		PRINT_RED("Error getsockname on new connection : Webserv can't create this new connection.")<<std::endl;
		return false;
	}
	this->addr = convertAddrBack(addr_tmp.sin_addr.s_addr);
	this->port = ntohs(addr_tmp.sin_port);
	// PRINT_RED("Test du port et adresse apres accept : ")<<this->addr<<" : "<<this->port<<std::endl;
	return true;
}
