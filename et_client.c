#include <stdio.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <strings.h>
#include <string.h>

#include <sys/epoll.h>
#include <errno.h>

#define SERV_PORT 1234
#define MAX_EVENT 1

int main(int argc, char *argv[])
{
    int ret = 0;
    int nfds = 0;
    int sockfd = 0;
    int epfd = 0;
    struct sockaddr_in serveraddr;
	struct epoll_event ev, events;

    sockfd = socket(AF_INET , SOCK_STREAM|SOCK_NONBLOCK|SOCK_CLOEXEC , 0);
//    sockfd = socket(AF_INET , SOCK_STREAM , 0);
    if(sockfd < 0)
    {
        perror("socket failed");
        return 0;
    }

    bzero(&serveraddr , sizeof(serveraddr));
    char *local_addr = "10.1.93.43";
    inet_aton(local_addr, &(serveraddr.sin_addr));
    serveraddr.sin_port = htons(SERV_PORT);
    serveraddr.sin_family = AF_INET;

    ret = connect(sockfd , (const struct sockaddr *)&serveraddr , sizeof(struct sockaddr_in));
    if(ret < 0 && ret != EINPROGRESS){
        perror("connect failed:");
        goto out;
    }

    epfd = epoll_create(MAX_EVENT);
    if(epfd < 0)
    {
        perror("epoll failed:");
        goto close;
    }
	
    ev.data.fd = sockfd;
//	ev.events = EPOLLOUT | EPOLLIN | EPOLLET;
	ev.events = EPOLLOUT | EPOLLIN;

	epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

    int i;
    while(1)
    {
        nfds = epoll_wait(epfd , &events , MAX_EVENT , -1);
        if(nfds < 0)
        {
            perror("epoll wait failed:");
            break;
        }

        if(events.data.fd == sockfd)
        {
            int val = 0;
            int len = sizeof(int);
            if( (ret = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &val, &len)) < 0 )
            {
                perror("getsockopt failed:");
                break;
            }
            if(val != 0)
            {
                perror("connect failed:");
                break;
            }

            char buf[] = "Test aaaaa";
            if(send(sockfd , buf , strlen(buf) , 0) < 0){
                perror("send failed");
                break;
            }
        }
    }

close:
    close(sockfd);

out:
    return 0;
}
