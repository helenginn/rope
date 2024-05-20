// vagabond
// Copyright (C) 2022 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#include "Socket.h"
#include "Dictator.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

Socket::Socket(Dictator *dictator, unsigned short port)
{
	_dictator = dictator;
	setResponder(_dictator);
	_port = port;
}

bool Socket::operator()()
{
	char buffer[BUFSIZ];
	char protoname[] = "tcp";
	struct protoent *protoent;
	int enable = 1;
	int newline_found = 0;
	int server_sockfd, client_sockfd;
	socklen_t client_len;
	ssize_t nbytes_read;
	struct sockaddr_in client_address, server_address;
	unsigned short server_port = _port;

	protoent = getprotobyname(protoname);
	if (protoent == NULL)
	{
		perror("getprotobyname");
		return false;
	}

	server_sockfd = socket(AF_INET,
	                       SOCK_STREAM,
	                       protoent->p_proto
	                       /* 0 */);
	if (server_sockfd == -1)
	{
		perror("socket");
		return false;
	}

	if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0)
	{
		perror("setsockopt(SO_REUSEADDR) failed");
		return false;
	}

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(server_port);
	if (bind(server_sockfd, (struct sockaddr*)&server_address,
	         sizeof(server_address)) == -1)
	{
		perror("bind");
		return false;
	}

	if (listen(server_sockfd, 5) == -1)
	{
		perror("listen");
		return false;
	}

	fprintf(stderr, "listening on port %d\n", server_port);

	while (1)
	{
		std::cout << "running" << std::endl;
		client_len = sizeof(client_address);
		client_sockfd = accept(server_sockfd,
		                       (struct sockaddr*)&client_address,
		                       &client_len);
		while ((nbytes_read = read(client_sockfd, buffer, BUFSIZ)) > 0)
		{
			printf("received:\n");
			write(STDOUT_FILENO, buffer, nbytes_read);
			if (buffer[nbytes_read - 1] == '\n')
			{
				const char *heard = "RoPE acknowledges: ";
				write(client_sockfd, heard, strlen(heard));
				write(client_sockfd, buffer, nbytes_read);
				std::string str = buffer;
				str.pop_back();
				buffer[nbytes_read - 1] = '\0';
				sendResponse("arg", &str);
			}
		}

		std::cout << "done" << std::endl;

		close(client_sockfd);
	}

	return true;
}

