/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/16 13:17:47 by clbernar          #+#    #+#             */
/*   Updated: 2024/06/14 17:59:43 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Handler.hpp"

// valgrind --leak-check=full --track-fds=yes

int	main(int argc, char **argv)
{
	(void)argv;
	if (argc != 2)
		std::cout<<"This program requires one and only one argument."<<std::endl;
	else
	{
		Handler	handler;
		signal(SIGINT, signalHandler);
		signal(SIGQUIT, signalHandler);
		// signal(SIGPIPE, SIG_IGN);
		handler.initTestConfig();//PARSING

		handler.initServer();
		handler.launchServer();
		// clean ??
	}
	// Checker le code d'erreur de l'objet Server pour retourner le bon code d'erreur
	return 0;
}
