/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/22 16:21:15 by clbernar          #+#    #+#             */
/*   Updated: 2024/05/01 18:39:31 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

// This functions convert a string address IPv4 in unsigned long to put it in a sockaddr_in struct
// Same role as inet_pton() function that converts text to binary form (that we can't use in the project)
// It uses std::istringstream to split the string with getline from each "."
// It stocks in the atoi conversion of each part in ip_bytes (unsigned char tab)
// And it shitfs the bytes to reorganise in unsigned long format
unsigned long	convertAddr(std::string to_convert)
{
	// CHECK mauvais format de string  => return 0
	std::istringstream	iss(to_convert);
	std::string	token;
	unsigned char	ip_bytes[4];
	int i = 0;

	while (std::getline(iss, token, '.'))
		ip_bytes[i++] = atoi(token.c_str());

	// for (int j = 0; j < 4; ++j)
	// 	std::cout<<ip_bytes[j]<<" ";
	// std::cout<<std::endl;

	unsigned long	ip_addr =
		(ip_bytes[0] << 24) |
		(ip_bytes[1] << 16) |
		(ip_bytes[2] << 8) |
		ip_bytes[3];
	return ip_addr;
}

/***********************************************************************************************************
 *                                                                                                         *
 *                                                  SIGNAL                                                 *
 *                                                                                                         *
 ***********************************************************************************************************/

// EXPLICATION 1 : La gestion de memoire des vector semble complexe. Apres avoir close toutes les sockets
// J'avais des still recheable (alors que je n'ai rien alloue dynamiquement) qui provenait donc de mes vector
// Pour liberer la memoire allouee par le vecteur lui meme, la solution etait d'utiliser shrink_to_fit() pour
// l'espace non necessaire apres avoir clear le vector mais cette methode des vectors est issue de C++11.
// Pour regler ce soucis je suis passer par un vector temporaire en utilisant "std::vector<Config>()" qui permet
// de creer un objet temporaire qui ne sera donc pas stocker dans une variable.
// Je swap ce vector vide avec mon vector qui se retrouve donc dans mon vector temporaire
// Le destructeur du vector est alors appele sur mon vector et ma variable globale contient desormais un vector vide
// EXPLICATION 2 : Par souci de simplicite et parce que le but du projet est bien loin de la gestion des signaux,
// j'ai choisi de gerer les signaux en dehors de ma classe Handler en passant une structure globale pour avoir acces
// aux donnees a liberer ou a close dans mon gestionnaire de signal.
// La solution plus propre aurait ete de rendre ma classe Handler Singleton (design pattern ou patron de conception)
// c'est a dire la rendre instanciable qu'une seule fois comme ce qui est amene a etre le cas.
// Cette methode implique d'avoir un constructeur privee, une methode d'acces statique qui renvoie l'instance unique
// de la classe, une instance statique cree a l'interieur de la methode d'acces statique et un pointeur vers l'instance
// unique a travers lequel j'aurais pu acceder a mes donnees dans ma gestion des signaux.
// Cela permet d'appeler la methode statique d'acces depuis n'importe ou pour avoir acces a mes elements.
// Cette methode aurait aussi pu s'utiliser sur une autre classe a l'interieur de ma classe Handler.
void	signalHandler(int signal_num)
{
	extern clearFromHanlder global;

	std::vector<Config>().swap(*global.global_config);
	if (signal_num == SIGINT)
		std::cout<<"Signal SIGINT received. Cleaning in progress..."<<std::endl;
	else if (signal_num == SIGQUIT)
		std::cout<<"Signal SIGQUIT received. Cleaning in progress..."<<std::endl;
	std::cout<<"Cleaning listenning sockets"<<std::endl;
	std::for_each(global.global_listen_connection->begin(), global.global_listen_connection->end(), &closeSocket);
	std::vector<Connection>().swap(*global.global_listen_connection);
	std::cout<<"Cleaning http communication sockets\n"<<std::endl;
	std::for_each(global.global_http_connection->begin(), global.global_http_connection->end(), &closeSocket);
	std::vector<Connection>().swap(*global.global_http_connection);
	if (close(*(global.global_epoll_fd)) == -1)
		PRINT_RED("Closing epoll_fd failed ")<<strerror(errno)<<std::endl;
	else
		std::cout<<"Epoll_fd closed"<<std::endl;
	exit(signal_num);
}

// This functions is used on a for_each to close socket on each Connection object of a vector
void	closeSocket(Connection objet)
{
	if (close(objet.getSocket()) == -1)
	{
		PRINT_RED("Closing connection failed ")<<"on socket "<<objet.getSocket()<<" : "<<strerror(errno)<<std::endl;
		// Gestion de l'erreur ?
	}
	std::cout<<"socket "<<objet.getSocket()<<" closed"<<std::endl;
}

