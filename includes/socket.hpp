/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tsilveir <tsilveir@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/16 12:20:37 by tsilveir          #+#    #+#             */
/*   Updated: 2026/02/16 12:28:44 by tsilveir         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP
# define SOCKET_HPP

// For socket
# include <arpa/inet.h>
# include <netdb.h>
# include <netinet/in.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/epoll.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <unistd.h>

# define BACKLOG 10

int		create_listening_socket(int port, const char *ip);
int		accept_conn_socket(int listen_sock);
int		setnonblocking(int sock);

// epoll
void	add_socket_epoll(int epollfd, int conn_sock);
void	add_cgifd_epoll(int epollfd, int cgifd);
void	remove_socket_epoll(int epollfd, int conn_sock);
void	change_socket_epollout(int epollfd, int conn_sock);
void	change_socket_epollin(int epollfd, int conn_sock);

#endif
