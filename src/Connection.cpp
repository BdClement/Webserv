/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/25 15:38:08 by clbernar          #+#    #+#             */
/*   Updated: 2024/04/26 18:45:20 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"

Connection::Connection()
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
}

Connection::~Connection()
{
	// std::cout<<"Connection destructor called"<<std::endl;
}

Connection& Connection::operator=(Connection const & equal)
{
	if (this != &equal)
	{
		this->socket = equal.socket;
		this->interface = equal.interface;
		this->event = equal.event;
	}
	return *this;
}