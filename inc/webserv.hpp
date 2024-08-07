/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/16 11:52:42 by clbernar          #+#    #+#             */
/*   Updated: 2024/06/28 11:43:48 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// Ce fichier est le header principal des fonctions et define du projet (hors classes)

#pragma once

// C
#include <unistd.h> // Pour sleep
#include <sys/epoll.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>

// C++ Versions of C Library
#include <cstdlib>
#include <csignal>
#include <cstring>
#include <cerrno>
#include <ctime>
#include <climits>
#include <cstdio>

// C++
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>

// C++ STL
#include <vector>
#include <algorithm>
#include <map>

// NB VALUES
#define MAX_EVENTS		20
#define URI_SIZE_MAX	2048
// #define BUFFER_SIZE		1
#define BUFFER_SIZE 	1024 //Minimum 6 checkChunked
//Mettre en place de taille pour chaque element ??
//#define REQUESTLINE_MAX_SIZE 8192 // 8Ko =? 414 Request-URI Too Long
#define HEADERLINE_MAX_SIZE 8192 // 8Ko => 431 Request Fields Too Large
#define HEADERTOTAL_MAX_SIZE 16384 //16Ko => 431 Request Fields Too Large
// #define REQUEST_MAX_SIZE 65536 //64Ko => 413 Payload Too Large
#define MEGAOCTET	1048576

#define TIMEOUT 30

// SHORTCUT
#define mapString		std::map<std::string, std::string>
#define mapIntString	std::map<int, std::string>

// DISPLAY
#define RED_COLOR "\033[1;31m"
#define GREEN_COLOR "\033[0;32m"
#define RESET "\033[0m"
#define PRINT_RED(text) std::cerr<<RED_COLOR<<text<<RESET
#define PRINT_GREEN(text) std::cout<<GREEN_COLOR<<text<<RESET

