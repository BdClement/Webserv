/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/24 13:59:57 by clbernar          #+#    #+#             */
/*   Updated: 2024/05/01 18:24:24 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

Config::Config(const std::string& addr, unsigned int port, const std::string& name) : listen_addr(addr), listen_port(port), server_name(name)
{
	// std::cout<<"Config constructor called"<<std::endl;
}

Config::Config(Config const& asign)
{
	// std::cout<<"Config copy constructor called"<<std::endl;
	listen_addr = asign.listen_addr;
	listen_port = asign.listen_port;
	server_name = asign.server_name;
}

Config::~Config()
{
	// std::cout<<"Config destructor called"<<std::endl;
}

Config& Config::operator=(Config const & equal)
{
	// std::cout<<"Config assignation operator called"<<std::endl;
	// A faire plutard
	if (this != &equal)
	{
		this->listen_addr = equal.listen_addr;
		this->listen_port = equal.listen_port;
		this->server_name = equal.server_name;
	}
	return *this;
}
