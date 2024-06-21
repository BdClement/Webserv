/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFile.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bmirlico <bmirlico@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/19 17:24:48 by bmirlico          #+#    #+#             */
/*   Updated: 2024/05/22 22:48:32 by bmirlico         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

ConfigFile::ConfigFile(void) : _filePath("") {}

ConfigFile::ConfigFile(const std::string path) : _filePath(path) {}

ConfigFile::ConfigFile(const ConfigFile &cpy)
{
	*this = cpy;
}

ConfigFile& ConfigFile::operator=(const ConfigFile &rhs)
{
	if (this != &rhs)
		this->_filePath = rhs.getFilePath();
	return (*this);
}

ConfigFile::~ConfigFile(void) {}

int ConfigFile::getTypeFilePath(std::string const path)
{
	struct stat buff;
	int result;

	result = stat(path.c_str(), &buff);
	if (result == 0)
	{
		if (buff.st_mode & S_IFREG)
			return (1);
		else if (buff.st_mode & S_IFDIR)
			return (2);
		else
			return (3);
	}
	else
		return (0);
}

int ConfigFile::fileExistsandReadable(std::string const path)
{
	if (access(path.c_str(), F_OK) == 0 && access(path.c_str(), R_OK) == 0)
		return (1);
	else if (access(path.c_str(), F_OK) == 0 && access(path.c_str(), R_OK) < 0)
		return (0);
	else
		return (-1);
}

// fonction qui permet de tester l'existence d'un fichier via chemin absolu ou relatif
int ConfigFile::filePathExistsandReadable(std::string const path, std::string const index)
{
	if (getTypeFilePath(index) == 1 && fileExistsandReadable(index) == 1)
		return (1);
	if (getTypeFilePath(path + index) == 1 && fileExistsandReadable(path + index) == 1)
		return (1);
	return (0);
}

std::string ConfigFile::readConfigFile(std::string path)
{
	std::ifstream	configFile(path.c_str());

	if (!configFile || !configFile.is_open())
		return (NULL);
	std::stringstream streamInput;
	streamInput << configFile.rdbuf();
	return (streamInput.str());
}

std::string ConfigFile::getFilePath(void) const
{
	return (this->_filePath);
}