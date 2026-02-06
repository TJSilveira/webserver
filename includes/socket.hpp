#ifndef SOCKET_HPP
#define SOCKET_HPP

// For socket
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BACKLOG 10

int create_listening_socket(int port, const char *ip);
int accept_conn_socket(int listen_sock);
int	setnonblocking(int sock);

// epoll
void add_socket_epoll(int epollfd, int conn_sock);
void remove_socket_epoll(int epollfd, int conn_sock);
void change_socket_epollout(int epollfd, int conn_sock);

#endif