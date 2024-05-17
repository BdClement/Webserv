/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/17 16:05:07 by clbernar          #+#    #+#             */
/*   Updated: 2024/05/17 18:09:37 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "Request.hpp"

Response::Response()
{
	// std::cout<<"Response constructor called"<<std::endl;
}

Response::Response(Response const& asign)
{
	// std::cout<<"Response copy constructor called"<<std::endl;
	(void)asign;
}

Response::~Response()
{
	// std::cout<<"Response destructor called"<<std::endl;
}

Response& Response::operator=(Response const & equal)
{
	if (this != &equal)
	{
		(void)equal;
	}
	return *this;
}

void	Response::generateResponse(std::vector<Config> & m_config, Request & request)
{
	(void)m_config;
	if (request.m_error_code != 0)
		generateErrorResponse(request.m_error_code);
	else
	{
		// Penser a la logique de reponse
			// - Logique de generation de ReponseLine (Pas de binaire)
			// - Logique de Generation de Headers (Pas de binaire)
			// - Logique de generation de Body (peu etre binaire)
	}
}

void	Response::generateErrorResponse(int error_code) const
{
	(void)error_code;
}

// Pour les errors, la fonction peut recvoir le code d'erreur
// Reflechir a a parti de quand on dit que cest valider ?
void	Response::generateStatusLine()
{
	// (void);
	//HTTP/1.1 CODE Commentaire
}
