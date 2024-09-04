#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct s_client
{
    int     id;
    char    msg[300000];
} t_client;

t_client    clients[1024];
int         gid = 0,            maxfd = 0;
fd_set      read_set,           write_set,          current_set;
char        send_bfr[300000],   recv_bfr[300000];

void    err(char *msg)
{
    if (msg)
        write(2, msg, strlen(msg));
    else
        write(2, "Fatal error", 11);
    write(2, "\n", 1);
    exit (1);
}

void    send_to_all(int except)
{
    for (int fd = 0; fd <= maxfd; fd++)
    {
        if (FD_ISSET(fd, &write_set) && fd != except)
            if (send(fd, send_bfr, strlen(send_bfr), 0) == -1)
                err(NULL);
    }
}

int     main(int ac, char **av)
{
    if (ac != 2)
        err("Wrong number of arguments");

    int                 sockfd;
    socklen_t           len;
	struct sockaddr_in  servaddr; 

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1)
        err(NULL);
    maxfd = sockfd;

    FD_ZERO(&current_set);
    FD_SET(sockfd, &current_set);
    bzero(clients, sizeof(clients));
    bzero(&servaddr, sizeof(servaddr));
    
    servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(av[1]));

    if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) == -1 || listen(sockfd, 100) == -1)
        err(NULL);

    while (1)
    {
        read_set = write_set = current_set;
        if (select(maxfd + 1, &read_set, &write_set, 0, 0) == -1) continue;

        for (int fd = 0; fd <= maxfd; fd++)
        {
            if (FD_ISSET(fd, &read_set))
            {
                if (fd == sockfd)
                {
                    // connexion
                    int clientfd = accept(sockfd, (struct sockaddr *)&servaddr, &len);
                    if (clientfd == -1) continue;
                    if (clientfd > maxfd) maxfd = clientfd;
                    clients[clientfd].id = gid++;
                    FD_SET(clientfd, &current_set);
                    sprintf(send_bfr, "server: client %d just arrived\n", clients[clientfd].id);
                    send_to_all(clientfd);
                }
                else
                {
                    // Déconnexion ou message
                    int ret = recv(fd, recv_bfr, sizeof(recv_bfr), 0);
                    if (ret <= 0)
                    {
                        // déconnexion
                        sprintf(send_bfr, "server: client %d just left\n", clients[fd].id);
                        send_to_all(fd);
                        FD_CLR(fd, &current_set);
                        close(fd);
                        bzero(&clients[fd].msg, strlen(clients[fd].msg));
                    }
                    else
                    {
                        // Nouveau message
                        for (int i = 0, j = strlen(clients[fd].msg); i < ret; i++, j++)
                        {
                            clients[fd].msg[j] = recv_bfr[i];
                            if (clients[fd].msg[j] == '\n')
                            {
                                // fin du message reçu
                                clients[fd].msg[j] = '\0';
                                sprintf(send_bfr, "client %d: %s\n", clients[fd].id, clients[fd].msg);
                                send_to_all(fd);
                                bzero(&clients[fd].msg, strlen(clients[fd].msg));
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