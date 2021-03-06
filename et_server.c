#include <stdio.h>

//for socket api
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

//for fork
#include <unistd.h>

//for bzero
#include <strings.h>
#include <string.h>

//for epoll
#include <sys/epoll.h>

#define SERV_PORT 1234
#define MAXLINE 1024
#define LISTENQ 128

//int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

int main()
{
	int listenfd = 0;
	int connfd = 0;
	int epfd = 0;
	struct sockaddr addr;
	struct sockaddr_in clientaddr;
	struct sockaddr_in serveraddr;
	int clientaddr_size = 0;
	pid_t pid = 0;
	struct epoll_event ev events[10];

	//    listenfd = socket(AF_INET , SOCK_STREAM|SOCK_NONBLOCK|SOCK_CLOEXEC , 0);
	listenfd = socket(AF_INET , SOCK_STREAM , 0);
	if(listenfd < 0){
		perror("socket create failed:");
		return 0;
	}

	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	char *local_addr = "10.1.93.43";
	inet_aton(local_addr, &(serveraddr.sin_addr));
	serveraddr.sin_port = htons(SERV_PORT);
	bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

	listen(listenfd, LISTENQ);

	epfd = epoll_create(1);
	if(epfd < 0)
	{
		perror("epoll_create failed:");
		return 0;
	}
	//int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

	ev.data.fd = listenfd;
	ev.events = EPOLLIN;

	epoll_ctl(epfd , EPOLL_CTL_ADD , listenfd , &ev);

	int nfds = 0;
	int i = 0;
	while(1) {
		//int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

		nfds = epoll_wait(epfd , events , 10 , -1);
		if(nfds < 0)
		{
			perror("epoll_wait failed:");
			break;
		}

		for(i = 0; i < nfds; i++)
		{
			if(events[i].data.fd == listenfd)
			{
				clientaddr_size = sizeof(struct sockaddr_in);
				connfd = accept(listenfd, (struct sockaddr *) &clientaddr,
						&clientaddr_size);
				if (connfd == -1){
					perror("failed to accept:");
					continue;
				}

				memset(&ev , 0 , sizeof(struct event_poll));
				ev.data.fd = connfd;
				ev.events = EPOLLIN;
				epoll_ctl(epfd , EPOLL_CTL_ADD , connfd , &ev);
			}else if(events[i].events & EPOLLIN) {

				if( !(pid = fork()) ){
					int n = 0;
					char buf[MAXLINE];
					memset(buf , 0 , MAXLINE);

					while ( (n = recv(connfd, buf,  MAXLINE,0)) > 0)  {
						//printf("%s","String received from and resent to the client:");
						//printf("%s",buf); //puts(buf);
						//send(connfd, "Send OK!\n", 15, 0);
						puts(buf);
						//say(connfd,"Send OK!\n");
						//say(connfd,buf);
					}

					if( n < 0 ){
						perror("recv failed:");
					}

					close(connfd);
					return 0;
				}else if(pid > 0){
					close(connfd);
				}
			}
		}
	}

	return 0;
}
