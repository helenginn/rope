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

#include "../utils/os.h"
#include "Socket.h"
#include "Dictator.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gemmi/cif.hpp>

#ifdef OS_UNIX
	#include <arpa/inet.h>
	#include <netdb.h> /* getprotobyname */
	#include <netinet/in.h>
	#include <sys/socket.h>
	#include <unistd.h>
#else
#ifdef OS_WINDOWS
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <windows.h>
	#pragma comment(lib, "ws2_32.lib")
#endif
#endif

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
#ifdef OS_UNIX
	ssize_t nbytes_read;
#else
#ifdef OS_WINDOWS
	int nbytes_read;
#endif
#endif
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

#ifdef OS_UNIX
	if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0)
#else
#ifdef OS_WINDOWS
		if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&enable), sizeof(enable)) < 0)
#endif
#endif
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
#ifdef OS_UNIX
		while ((nbytes_read = read(client_sockfd, buffer, BUFSIZ)) > 0)
#else
#ifdef OS_WINDOWS
		while ((nbytes_read = recv(server_sockfd, buffer, BUFSIZ, 0)) > 0)
#endif
#endif
		{
			printf("received:\n");
#ifdef OS_UNIX
			write(STDOUT_FILENO, buffer, nbytes_read);
#else
#ifdef OS_WINDOWS
			send(1, buffer, nbytes_read, 0);
#endif
#endif
			if (buffer[nbytes_read - 1] == '\n')
			{
				const char *heard = "RoPE acknowledges: ";
#ifdef OS_UNIX
				write(client_sockfd, heard, strlen(heard));
				write(client_sockfd, buffer, nbytes_read);
#else
#ifdef OS_WINDOWS
				send(client_sockfd, heard, strlen(heard), 0);
				send(client_sockfd, buffer, nbytes_read, 0);
#endif
#endif
				std::string str = buffer;
				str.pop_back();
				buffer[nbytes_read - 1] = '\0';
				sendResponse("arg", &str);
			}
		}

		std::cout << "done" << std::endl;

#ifdef OS_UNIX
		close(client_sockfd);
#else
#ifdef OS_WINDOWS
		closesocket(client_sockfd);
#endif
#endif
	}

	return true;
}

