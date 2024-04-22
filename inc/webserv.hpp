/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/16 11:52:42 by clbernar          #+#    #+#             */
/*   Updated: 2024/04/22 18:08:28 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// Ce fichier est le header principal des fonctions et define du projet (hors classes)

#ifndef __WEBSERV__HPP
#define __WEBSERV__HPP

#include <string>
#include <iostream>
#include <sstream>
#include <cstdlib>

#include "Handler.hpp"

unsigned long	convert_addr(std::string to_convert);

#endif
