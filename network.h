#ifndef _NETWORK_H
#define _NETWORK_H

int host_server(const char *port);

int connect_to_sever(const char *address, const char *port);

int read_network_line(int fd, char *input_buffer);

int write_network_line(int fd, char *input_buffer, int size);

#endif
