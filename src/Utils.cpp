/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/22 16:21:15 by clbernar          #+#    #+#             */
/*   Updated: 2024/04/22 17:09:15 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

// This functions convert a string address IPv4 in unsigned long to put it in a sockaddr_in struct
// Same role as inet_pton() function that converts text to binary form (that we can't use in the project)
unsigned long	convert_addr(std::string to_convert)
{
	// CHECK mauvais format de string  => return 0
	std::istringstream	iss(to_convert);
	std::string	token;
	unsigned char	ip_bytes[4];
	int i = 0;

	while (std::getline(iss, token, '.'))
		ip_bytes[i++] = atoi(token.c_str());

	for (int j = 0; j < 4; ++j)
		std::cout<<ip_bytes[j]<<" ";
	std::cout<<std::endl;
	unsigned long	ip_addr =
		(ip_bytes[0] << 24) |
		(ip_bytes[1] << 16) |
		(ip_bytes[2] << 8) |
		ip_bytes[3];
	return ip_addr;
}
