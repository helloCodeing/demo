#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EVENTS 1024

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        // port error
        printf("please enter port\n");
        return -1;
    }
    uint64_t port = atoi(argv[1]);
    if (port > 65535 || port < 1024)
    {
        // invalid port
        return -1;
    }
    int fd;
    fd = socket(AF_INET,SOCK_STREAM,0);
    if (fd < 0)
    {
        printf("socket failed\n");
        return fd;
    }
    int options = 1;
    int ret = 0;
    ret = setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&options,sizeof options);
    if(ret < 0)
    {
        printf("setsockopt failed\n");
        return ret;
    }
    struct sockaddr_in self;
    self.sin_family = AF_INET;
    self.sin_addr.s_addr = htonl(INADDR_ANY);
    self.sin_port = htons(port);
    ret = bind(fd,(const struct sockaddr*)&self,sizeof self);
    if (ret)
    {
        printf("bind failed\n");
        return ret;
    }
    listen(fd,88);

    int epollfd = epoll_create(MAX_EVENTS);
    if (epollfd < 0)
    {
        printf("create epoll failed\n");
        return epollfd;
    }

    struct epoll_event ev;
    struct epoll_event events[MAX_EVENTS];
    // listen event
    ev.events = EPOLLIN;
    ev.data.fd = fd;

    ret = epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);
    if (ret < 0)
    {
        printf("epoll_ctl failed\n");
        return ret;
    }
    int nfds = 0;
    while(1)
    {
        nfds = epoll_wait(epollfd,events,MAX_EVENTS,-1);
        if (nfds < 0)
        {
            printf("epoll_wait failed\n");
            break;
        }
        for(int i = 0; i < nfds; ++i)
        {
            if (events[i].data.fd == fd)
            {
                struct sockaddr_in cliaddr;
                int len;
                int cli = accept(fd,(struct sockaddr*)&cliaddr,(socklen_t*)&len);
                if(cli == -1)
                {
                    continue;
                }
                
                ev.events = EPOLLIN;
                ev.data.fd = cli;
                ret = epoll_ctl(epollfd,EPOLL_CTL_ADD,cli,&ev);
                if (ret < 0)
                {
                    printf("epoll_ctl failed\n");
                }
                continue;
            }
            else if(events[i].events == EPOLLIN)
            {
                char buf[1024] = {0};
                int length = read(events[i].data.fd, buf, sizeof buf);
                if (length < 0)
                {
                    printf("read failed\n");
                    continue;
                }
                else if (length == 0)
                {
                    printf("client over\n");
                    ev.events = EPOLLOUT;
                    ev.data.fd = events[i].data.fd;
                    ret = epoll_ctl(epollfd,EPOLL_CTL_DEL,events[i].data.fd,&ev);
                    if (ret < 0)
                    {
                        printf(" -- > epoll_ctl failed\n");
                    }
                    continue;
                }
                else
                {
                    printf("=============== read message ===============\n");
                    printf("%s\n",buf);
                    printf("=============== read message ===============\n");
                }
            }
            else if(events[i].events == EPOLLOUT)
            {
                printf("write event\n");
            }
        }
    }

    close(fd);
    close(epollfd);
    return 0;
}
