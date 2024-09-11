#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
char b[1024],m[1024];int i[1024],g,M,s,f,n,r,x;fd_set S,R,W;
void e(char*m){write(2,m,strlen(m));exit(1);}
int main(int c,char**v){if(c!=2)e("Wrong number of arguments\n");struct sockaddr_in a;
if((s=socket(AF_INET,SOCK_STREAM,0))<0)e("Fatal error\n");M=s;FD_ZERO(&S);FD_SET(s,&S);
bzero(&a,sizeof(a));a.sin_family=AF_INET;a.sin_addr.s_addr=htonl(2130706433);
a.sin_port=htons(atoi(v[1]));if(bind(s,(struct sockaddr*)&a,sizeof(a))<0||listen(s,10)<0)e("Fatal error\n");
while(1){R=S;if(select(M+1,&R,0,0,0)<0)continue;for(f=0;f<=M;f++)if(FD_ISSET(f,&R)){x=f;
if(f==s){if((n=accept(s,0,0))<0)continue;if(n>M)M=n;i[n]=g++;FD_SET(n,&S);x=n;
sprintf(b,"server: client %d just arrived\n",i[n]);}else{
if((r=recv(f,m,1024,0))<=0){sprintf(b,"server: client %d just left\n",i[f]);FD_CLR(f,&S);close(f);i[f]=-1;}
else{for(int j=0,k=0;j<r;j++)if(m[j]=='\n'){m[k]=0;sprintf(b,"client %d: %s\n",i[f],m);k=0;}else m[k++]=m[j];}}
W=S;for(n=0;n<=M;n++)if(FD_ISSET(n,&W)&&n!=x&&n!=s&&i[n]!=-1)send(n,b,strlen(b),0);bzero(b,1024);break;}}}
