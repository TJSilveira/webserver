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
		return -1;
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

void add_socket_epoll(int epollfd, int conn_sock)
{
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = conn_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
		perror("epoll_ctl: problem connecting socket to epoll");
		exit(EXIT_FAILURE);
	}
}

void add_cgifd_epoll(int epollfd, int cgifd)
{
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLHUP;
	ev.data.fd = cgifd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, cgifd, &ev) == -1) {
		perror("epoll_ctl: problem connecting cgifd to epoll");
		exit(EXIT_FAILURE);
	}
}

void remove_socket_epoll(int epollfd, int conn_sock)
{
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = conn_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_DEL, conn_sock, &ev) == -1) {
		perror("epoll_ctl: problem deleting socket to epoll");
		exit(EXIT_FAILURE);
	}
}

void change_socket_epollout(int epollfd, int conn_sock)
{
	struct epoll_event ev;
	ev.events = EPOLLOUT | EPOLLET;
	ev.data.fd = conn_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_MOD, conn_sock, &ev) == -1) {
		perror("epoll_ctl: problem putting socket ready to write epoll");
		exit(EXIT_FAILURE);
	}
}

void change_socket_epollin(int epollfd, int conn_sock)
{
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = conn_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_MOD, conn_sock, &ev) == -1) {
		perror("epoll_ctl: problem putting socket ready to write epoll");
		exit(EXIT_FAILURE);
	}
}