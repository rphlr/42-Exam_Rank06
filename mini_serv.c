#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct s_client
{
	int		id;
	char	msg[300000];
} t_client;

t_client	clients[1024];
int			maxfd = 0,		gid = 0;
char		sd_bfr[300000],	rcv_bfr[300000];
fd_set		rd_set,			wrt_set,			curr_set;

void	err(char *msg)
{
	if (msg)
		write(2, msg, strlen(msg));
	else
		write(2, "Fatal error", 11);
	write(2, "\n", 1);
	exit (1);
}

void	send_to_all(int except)
{
	for (int fd = 0; fd <= maxfd; fd++)
	{
		if (FD_ISSET(fd, &wrt_set) && fd != except)
			if (send(fd, sd_bfr, strlen(sd_bfr), 0) == -1)
				err(NULL);
	}
}

int		main(int ac, char **av)
{
	if (ac != 2)
		err("Wrong number of arguments");
	
	int					sockfd;
	socklen_t			len = sizeof(struct sockaddr_in);
	struct sockaddr_in	servaddr; 

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1)
		err(NULL);
	maxfd = sockfd;

	FD_ZERO(&curr_set);
	FD_SET(sockfd, &curr_set);
	bzero(clients, sizeof(clients));
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(av[1])); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) == -1 || listen(sockfd, 100) == -1)
		err(NULL);

	while (1)
	{
		rd_set = wrt_set = curr_set;
		if (select(maxfd + 1, &rd_set, &wrt_set, 0, 0) == -1) continue;

		for (int fd = 0; fd <= maxfd; fd++)
		{
			if (FD_ISSET(fd, &rd_set))
			{
				if (fd == sockfd)
				{
					// Connection
					int connfd = accept(sockfd, (struct sockaddr *)&servaddr, &len);
					if (connfd == -1) continue;
					if (connfd > maxfd) maxfd = connfd;
					clients[connfd].id = gid++;
					FD_SET(connfd, &curr_set);
					sprintf(sd_bfr, "server: client %d just arrived\n", clients[connfd].id);
					send_to_all(connfd);
				}
				else
				{
					// Disconnection or message
					int ret = recv(fd, &rcv_bfr, sizeof(rcv_bfr), 0);
					if (ret <= 0)
					{
						// Disconnection
						sprintf(sd_bfr, "server: client %d just left\n", clients[fd].id);
						send_to_all(fd);
						FD_CLR(fd, &curr_set);
						close(fd);
						bzero(clients[fd].msg, strlen(clients[fd].msg));
					}
					else
					{
						// Message
						for (int i = 0, j = strlen(clients[fd].msg); i < ret; i++, j++)
						{
							clients[fd].msg[j] = rcv_bfr[i];
							if (clients[fd].msg[j] == '\n')
							{
								clients[fd].msg[j] = '\0';
								sprintf(sd_bfr, "client %d: %s\n", clients[fd].id, clients[fd].msg);
								send_to_all(fd);
								bzero(clients[fd].msg, strlen(clients[fd].msg));
								j = -1;
							}
						}
					}
				}
				break;
			}
		}
	}
	return (0);
}
