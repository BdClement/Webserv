/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 13:56:32 by clbernar          #+#    #+#             */
/*   Updated: 2024/05/01 18:26:04 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __CONFIG__HPP
#define __CONFIG__HPP

#include "webserv.hpp"

class Config
{
	public:
	Config(const std::string& addr, unsigned int port, const std::string& name);
	Config(Config const& asign);
	~Config();

	Config & operator=(Config const& equal);

	private:
	std::string	listen_addr;
	unsigned int	listen_port;
	std::string	server_name;
	friend class Handler;
};

#endif
