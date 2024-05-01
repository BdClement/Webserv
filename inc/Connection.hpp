/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 18:45:29 by clbernar          #+#    #+#             */
/*   Updated: 2024/05/01 18:27:09 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __CONNECTION__HPP
#define __CONNECTION__HPP

#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>

// #include "webserv.hpp"

class Connection
{
	public:
	Connection();
	Connection(Connection const& asign);
	~Connection();

	Connection & operator=(Connection const& equal);

	int	getSocket() const;

	private:

	int	socket;
	struct	sockaddr_in	interface;
	struct	epoll_event	event;
	friend class Handler;

};

#endif
