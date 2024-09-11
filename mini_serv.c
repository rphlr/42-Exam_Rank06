#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

char	buf[1024],	msg[1024];
int		id[1024],	max_fd,		server_fd,	client_fd,	new_fd,	readc,	next_id = 0;
fd_set	all_fds,	read_fds,	write_fds;

void fatal(char *s)
{
	write(2, s, strlen(s)); exit(1);
}

int main(int ac, char **av)
{
	struct sockaddr_in	addr;

	if (ac != 2)
		fatal("Wrong number of arguments\n");   
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		fatal("Fatal error\n");
	max_fd = server_fd;

	FD_ZERO(&all_fds);
	FD_SET(server_fd, &all_fds);
	bzero(&addr, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(2130706433); // 127.0.0.1
	addr.sin_port = htons(atoi(av[1]));

	if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		fatal("Fatal error\n");
	if (listen(server_fd, 10) < 0)
		fatal("Fatal error\n");

	while (1)
	{
		read_fds = all_fds;
		if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0)
			continue;
		
		for (client_fd = 0; client_fd <= max_fd; client_fd++) {
			if (FD_ISSET(client_fd, &read_fds))
			{
				if (client_fd == server_fd)
				{
					if ((new_fd = accept(server_fd, NULL, NULL)) < 0)
						continue;
					if (new_fd > max_fd)
						max_fd = new_fd;
					id[new_fd] = next_id++;
					FD_SET(new_fd, &all_fds);
					sprintf(buf, "server: client %d just arrived\n", id[new_fd]);
					client_fd = new_fd;
				}
				else
				{
					if ((readc = recv(client_fd, msg, 1024, 0)) <= 0)
					{
						sprintf(buf, "server: client %d just left\n", id[client_fd]);
						FD_CLR(client_fd, &all_fds);
						close(client_fd);
					}
					else
					{
						for (int i = 0, j = 0; i < readc; i++)
						{
							if (msg[i] == '\n')
							{
								msg[j] = 0;
								sprintf(buf, "client %d: %s\n", id[client_fd], msg);
								j = 0;
							} 
							else
								msg[j++] = msg[i];
						}
					}
				}
				write_fds = all_fds;
				for (int fd = 0; fd <= max_fd; fd++)
					if (FD_ISSET(fd, &write_fds) && fd != client_fd && fd != server_fd)
						send(fd, buf, strlen(buf), 0);
				bzero(buf, sizeof(buf));
				break;
			}
		}
	}
	return 0;
}
