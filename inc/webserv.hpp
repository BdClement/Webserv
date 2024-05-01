/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/16 11:52:42 by clbernar          #+#    #+#             */
/*   Updated: 2024/05/01 18:40:00 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// Ce fichier est le header principal des fonctions et define du projet (hors classes)

#ifndef __WEBSERV__HPP
#define __WEBSERV__HPP

#include <string>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <csignal>
#include <unistd.h>

#include "Handler.hpp"
#include "Config.hpp"
#include "Connection.hpp"
#define MAX_EVENTS 20

// Couleurs ANSI
#define RED_COLOR "\033[1;31m"
#define GREEN_COLOR "\033[0;32m"
#define RESET "\033[0m"

#define PRINT_RED(text) std::cerr<<RED_COLOR<<text<<RESET
#define PRINT_GREEN(text) std::cout<<GREEN_COLOR<<text<<RESET

class Config;

struct clearFromHanlder{
	std::vector<Config>		* global_config;
	std::vector<Connection> * global_listen_connection;
	std::vector<Connection> * global_http_connection;
	int	* global_epoll_fd;
};

unsigned long	convertAddr(std::string to_convert);

void			signalHandler(int signal_num);
void			closeSocket(Connection objet);


#endif
