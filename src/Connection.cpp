/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/25 15:38:08 by clbernar          #+#    #+#             */
/*   Updated: 2024/06/19 17:47:30 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"

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

