/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/16 13:17:47 by clbernar          #+#    #+#             */
/*   Updated: 2024/04/26 16:41:10 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

int	main(int argc, char **argv)
{
	(void)argv;
	if (argc != 2)
		std::cout<<"This program requires one and only one argument."<<std::endl;
	else
	{
		// std::cout<<"Ca marche"<<std::endl;
		Handler	handler;

		// handler.parsing_config();
		handler.init_test_config();
		handler.init_listen_socket();

		// handler.init_server();
		// handler.launch_server();

		// clean ??
	}
	// Checker le code d'erreur de l'objet Server pour retourner le bon code d'erreur
	return 0;
}
