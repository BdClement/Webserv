/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 18:45:29 by clbernar          #+#    #+#             */
/*   Updated: 2024/06/19 13:06:40 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "webserv.hpp"
#include "Request.hpp"
#include "Response.hpp"

class Connection
{
	public:
	Connection();
	Connection(Connection const& asign);
	~Connection();

	Connection & operator=(Connection const& equal);

	int	getSocket() const;
	void	closeRequestCGIPipe();

	private:

	int						socket;
	struct	sockaddr_in		interface;
	struct	epoll_event		event;
	class Request			request;
	class Response			response;
	time_t					last_active_time;

	friend class Handler;
	friend struct CompareSocket;
};
