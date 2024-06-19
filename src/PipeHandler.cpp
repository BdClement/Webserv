/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PipeHandler.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: clbernar <clbernar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 12:24:25 by clbernar          #+#    #+#             */
/*   Updated: 2024/06/19 13:14:27 by clbernar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::PipeHandler::PipeHandler() : m_bytes_sent(0), m_pid(0)
{
	// std::cout<<"Constructor PipeHandler called"<<std::endl;
	pipe_stdin[0] = 0;
	pipe_stdin[1] = 0;
	pipe_stdout[0] = 0;
	pipe_stdout[1] = 0;
}

Request::PipeHandler::~PipeHandler() // A voir dans le cas ou le pipe est deja close mais pasmis a 0 ?
{
	// std::cout<<"Destructor PipeHandler called"<<std::endl;
	// clear();
	// A verifier 2 seront deja close d'entree
	// if (pipe_stdin[0] != 0)
	// 	close(pipe_stdin[0]);
	// if (pipe_stdin[1] != 0)
	// 	close(pipe_stdin[1]);
	// if (pipe_stdout[0] != 0)
	// 	close(pipe_stdout[0]);
	// if (pipe_stdout[1] != 0)
	// 	close(pipe_stdout[1]);
}

Request::PipeHandler & Request::PipeHandler::operator=(PipeHandler const& equal)
{
	// std::cout<<"Copy operator PipeHandler called"<<std::endl;
	if (this != &equal)
	{
		this->pipe_stdin[0] = equal.pipe_stdin[0];
		this->pipe_stdin[1] = equal.pipe_stdin[1];
		this->pipe_stdout[0] = equal.pipe_stdout[0];
		this->pipe_stdout[1] = equal.pipe_stdout[1];
		this->m_bytes_sent = equal.m_bytes_sent;
		this->m_pid = equal.m_pid;
		this->request_method = equal.request_method;
		this->script_name = equal.script_name;
		this->query_string = equal.query_string;
		this->CType = equal.CType;
		this->CLength = equal.CLength;
		this->server = equal.server;
		this->ressource = equal.ressource;
		this->bin = equal.bin;
	}
	return *this;
}

void	Request::PipeHandler::closeAndReset(int &pipe)
{
	if (pipe != 0)
	{
		close(pipe);
		pipe = 0;
	}
}

void	Request::PipeHandler::clear()
{
	// std::cout<<"PipeHandler clear called"<<std::endl;
	closeAndReset(pipe_stdin[0]);
	closeAndReset(pipe_stdin[1]);
	closeAndReset(pipe_stdout[0]);
	closeAndReset(pipe_stdout[1]);
	m_bytes_sent = 0;
	m_pid = 0;
	request_method.clear();
	script_name.clear();
	query_string.clear();
	CType.clear();
	CLength.clear();
	server.clear();
	ressource.clear();
	bin.clear();
}
