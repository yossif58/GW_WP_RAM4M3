/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */

#ifndef __BOT_SOCKET_H__
#define __BOT_SOCKET_H__

#include "bot_platform.h"

/**
 * @brief the protocol type to connect
 */
typedef enum {
    bot_net_tcp = 0,
    bot_net_udp,
    bot_net_inv
}bot_net_proto_type_e;

/**
 * @brief tests to see if a file descriptor is part of the set
 *
 * @param[in] fd   the test file descriptor
 * @param[in] set  the set of file descriptor  
 * @return  0: the fd is not in the set; others: the fd is in the set
 */
int BOT_FD_ISSET(int fd, fd_set *set);

/**
 * @brief set a file descriptor for a set
 *
 * @param[in] fd  the set file descriptor
 * @param[in] set the set of file descriptor
 * @return NULL
 */
void BOT_FD_SET(int fd, fd_set *set);

/**
 * @brief Clear a set
 *
 * @param[in] set the set which will be cleared
 *
 * @return NULL
 */
void BOT_FD_ZERO(fd_set *set);

/**
 * @brief get socket errno.
 *
 * @param[in] NULL
 *
 * @return  socket errno
 */
int bot_socket_errno(void);

/**
 * @brief creates an endpoint for communication and returns a descriptor.
 *
 * @param[in] domain    a communication domain
 * @param[in] type      communication semantics
 * @param[in] protocol  a particular protocol to be used with the socket
 *
 * @return  On success, a file descriptor for the new socket is returned.
 *          On error, -1 is returned, and errno is set appropriately
 */
int bot_socket_open(int domain, int type, int protocol);

/**
 * @brief write data to socket.
 *
 * @param[in] sockfd  the file descriptor of the sending socket
 * @param[in] data    the message to send
 * @param[in] size    the message length
 *
 * @return  On success, these calls return the number of bytes sent.
 *          On error, -1 is returned, and errno is set appropriately.
 */
int bot_socket_write(int sockfd, const void *data, size_t size);

/**
 * @brief read data from a socket.
 *
 * @param[in] sockfd  the file descriptor of the receiving socket
 * @param[in] data    the buffer to store the received data
 * @param[in] len     the expected length of received data
 *
 * @return  the number of bytes received, or -1 if an error occurred.
 */
int bot_socket_read(int sockfd, void *data, size_t len);

/**
 * @brief  connect the socket referred to by the file descriptor sockfd to the address specified by addr.
 *
 * @param[in] sockfd   the file descriptor of the connecting socket
 * @param[in] addr     the  address  to  which are sent
 * @param[in] addrlen  the size of addr
 *
 * @return  On success, zero is returned.
 *          On error, -1 is returned, and errno is set appropriately.
 */
int bot_socket_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

/**
 * @brief  Monitor multiple file descriptors, waiting until one or more of the file descriptors become "ready" for some class of I/O operatio.
 *
 * @param[in] maxfdp1   this argument should be set to the highest-numbered file descriptor in any of the three sets, plus 1
 * @param[in] readset   the set of which will be watched to see if characters become available for reading
 * @param[in] writeset  the set of which will be watched to see if characters become available for write
 * @param[in] exceptset the set of will be watched for exceptional conditions
 * @param[in] timeout   the time should block waiting for a file descriptor to become ready
 *
 * @return  On success, zero is returned.
 *          On error, -1 is returned, and errno is set appropriately.
 */
int bot_socket_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout);

/**
 * @brief  Close the socket referred to by sockfd.
 *
 * @param[in] sockfd  the file descriptor of the closing socket
 *
 * @return  On success, zero is returned.
 *          On error, -1 is returned, and errno is set appropriately.
 */
int bot_socket_close(int sockfd);

/**
 * @brief  Shutdown the socket referred to by sockfd.
 *
 * @param[in] sockfd  the file descriptor of the shutdown socket
 * @param[in] how     the method to shutdown
 *
 * @return  On success, zero is returned.
 *          On error, -1 is returned, and errno is set appropriately.
 */
int bot_socket_shutdown(int socket, int how);

/**
 * @brief  connect the socket with host:port in the given protocol.
 *
 * @param[in] fd     the file descriptor of the connecting socket
 * @param[in] host   Host to connect to
 * @param[in] port   Port to connect to
 * @param[in] proto  Protocol: bot_net_tcp or bot_net_udp
 *
 * @return  On success, zero is returned.
 *          On error, -1 is returned, and errno is set appropriately.
 */

int bot_socket_app_connect(int *fd, const char *host, const char *port, bot_net_proto_type_e proto);


#endif /* __BOT_SOCKET_H__ */

