#include "network.h"

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "input.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/unistd.h>

const char *hostname = "0.0.0.0";

int host_server(const char *port)
{
	// Validate port.
	int port_num = atoi(port);
	if (port_num == 0) {
		printf("Invalid port provided\n");
		return 0;
	}

	// Create socket to listen on, use IP and TCP.
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	int yes_reuse = 1;
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes_reuse, sizeof(int));
	if (socket_fd == -1) {
		printf("socket creation failed...\n");
		return 0;
	}
	printf("Socket created\n");

	// Set up the TCP server
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	// IP protocol, set address and port
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(hostname);
	server_addr.sin_port = htons(port_num);

	// Bind socket with server details, expect 0 on success
	if ((bind(socket_fd, (const struct sockaddr *)&server_addr,
		  sizeof(server_addr))) != 0) {
		printf("socket bind failed...\n");
		return 0;
	}

	printf("Socket binded\n");

	// Listen on socket
	if ((listen(socket_fd, 20)) != 0) {
		printf("Listen failed\n");
		return 0;
	}

	char readable_hostname[INET6_ADDRSTRLEN];
	getnameinfo((struct sockaddr *)&server_addr, sizeof(server_addr),
		    readable_hostname, sizeof(readable_hostname), 0, 0,
		    NI_NUMERICHOST);
	printf("Server listening... (fd: %d) on: %s:%d\n", socket_fd,
	       readable_hostname, port_num);
	// Accept client connection
	struct sockaddr_in client_addr;
	unsigned int len = sizeof(client_addr);
	int connection_fd = accept(socket_fd, (struct sockaddr *)&client_addr, &len);
	if (connection_fd < 0) {
		printf("server acccept failed...\n");
		return 0;
	}
	fcntl(connection_fd, F_SETFL, O_NONBLOCK);
	printf("server acccept the client...\n");
	close(socket_fd);
	return connection_fd;
}

int connect_to_sever(const char *address, const char *port)
{
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1) {
		printf("socket creation failed...\n");
		return 0;
	}
	printf("Socket created..\n");

	// Populate sockaddr_in struct with server details
	int port_num = atoi(port);
	if (port_num == 0) {
		return 0;
	}
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(address);
	server_addr.sin_port = htons(port_num);

	printf("Connecting to server.....\n");
	if (connect(socket_fd, (struct sockaddr *)&server_addr,
		    sizeof(server_addr)) != 0) {
		printf("connection with the server failed...\n");
		return 0;
	}
	printf("connected to the server..\n");
	fcntl(socket_fd, F_SETFL, O_NONBLOCK);
	return socket_fd;
}

int read_network_line(int fd, char *input_buffer)
{
	fd_set read_fds;
	FD_ZERO(&read_fds);
	FD_SET(fd, &read_fds);
	int ret = select(fd + 1, &read_fds, NULL, NULL, NULL);
	if (ret < 1) {
		return -1;
	}
	int bytes_read = 0;
	while (bytes_read < INPUT_BUFFER_SIZE) {
		int read_chunk =
			read(fd, input_buffer + bytes_read, INPUT_BUFFER_SIZE - bytes_read);
		if (read_chunk <= 0) {
			break;
		}
		bytes_read += read_chunk;
		if (input_buffer[bytes_read] == '\n') {
			break;
		}
	}
	return bytes_read;
}

int write_network_line(int fd, char *input_buffer, int size)
{
	int bytes_written = send(fd, input_buffer, size, MSG_DONTWAIT);
	printf("%d (%d) bytes written (%s)\n", bytes_written, size, input_buffer);
	return bytes_written;
}
