/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/16 13:17:47 by clbernar          #+#    #+#             */
/*   Updated: 2024/06/28 15:21:47 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Handler.hpp"
#include "ConfigParser.hpp"


int	main(int argc, char **argv)
{
	if (argc > 2)
		std::cout<<"This program can't take more than one argument."<<std::endl;
	else
	{
		Handler	handler;
		signal(SIGINT, signalHandler);
		signal(SIGQUIT, signalHandler);
			if (!handler.initConfig(argc == 1 ? NULL : argv[1]))
				return 1;
		handler.initServer();
		handler.launchServer();
	}
	return 0;
}
