#include "../includes/socket.hpp"
#include <string>
#include <iostream>
#include <fcntl.h>

int create_listening_socket(int port, const char *ip)
{
	int sockfd;
	struct sockaddr_in addr;
	int yes = 1;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return -1;
	}

	// Set SO_REUSEADDR to allow immediate restart
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		close(sockfd);
		return -1;
	}

	if (setnonblocking(sockfd) == -1)
		return -1;

	memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		close(sockfd);
		perror("server: bind");
	}	

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}
	return (sockfd);
}


int accept_conn_socket(int listen_sock)
{
	int sockfd;
	struct sockaddr_in addr;
	socklen_t sin_size;

	sin_size = sizeof(addr);
	sockfd = accept(listen_sock, (struct sockaddr *) &addr, &sin_size);

	if (sockfd == -1) {
		perror("accept");
		exit(EXIT_FAILURE);
	}

	if (setnonblocking(sockfd) == -1)
		return -1;
	
	return (sockfd);
}

int	setnonblocking(int sock)
{
	// Set socket to be non-blocking
	int flags = fcntl(sock, F_GETFL, 0);
	if (flags == -1) {
		perror("Cannot get the socket flags");
		close(sock);
		return -1;
	}

	if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
		perror("Cannot set the socket to non-blocking");
		close(sock);
		return -1;
	}
	return 0;
}
