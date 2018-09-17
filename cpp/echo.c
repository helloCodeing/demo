#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>


#define MAX_EVENTS 1024

#define LOGINFO(fmt...) \
    do{ \
        fprintf(stderr,"%s %s  ",__DATE__,__TIME__); \
        fprintf(stderr,##fmt); \
        fprintf(stderr,"   %s  %d\n",__FUNCTION__,__LINE__); \
    }while(0)

#define RE_PORT 80

char host_name[1024] = {0};

void write_msg(int fd,char*msg)
{
    write(fd,msg,strlen(msg));
}

void show_char(char*msg)
{
    int length = strlen(msg);
    for(int i = 0;i < length; ++i)
    {
        LOGINFO("index :\t%d  value :\t%d",i,msg[i]);
    }
}

void parse_host(char*msg)
{
    char *p = msg;
    char *host = strstr(p,"Host:");
    char *name = strchr(host,'\n');
    memcpy(host_name,host+6,name - host -6 - 1);
    /*LOGINFO("host name   : %s",host_name);*/
}



void parse_msg(char*msg)
{
    char *p = msg;
    char *end = strchr(p,'\0');
    char *phttp = strstr(p,"http://");
    char *purl = phttp + 7;
    char *arg = strchr(purl,'/');
    int hlen = phttp - p;
    int len = end - arg;
    memcpy(phttp,arg,len);
    p[len + hlen] = '\0';
    /*LOGINFO("arg         : %s",p);*/
}

int  create_connect()
{
    struct sockaddr_in sock;
    int lisfd;
    int clifd;

    struct hostent *addr = gethostbyname(host_name);
    if(!addr)
    {
        LOGINFO("-----%s-------addr successful",host_name);
        return -1;
    }
    lisfd = socket(AF_INET,SOCK_STREAM,0);
    if(lisfd < 0)
    {
        LOGINFO("------------------socket create failed");
        return -1;
    }

    memset(&sock,0,sizeof sock);
    sock.sin_family = AF_INET;
    memcpy(&sock.sin_addr.s_addr,addr->h_addr_list[0],addr->h_length);
    sock.sin_port = htons(RE_PORT);

    clifd = connect(lisfd,(struct sockaddr *)&sock,sizeof sock);
    if (clifd < 0)
    {
        char ip[32] = {0};
        LOGINFO("-------------------------------------connet failed %s",inet_ntop(addr->h_addrtype,addr->h_addr_list[0],ip,sizeof ip));
        return -1;
    }
    return lisfd;
}

int epoll_ctrl(int epfd,int type,int fd)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    int ret = epoll_ctl(epfd,type,fd,&ev);
    if (ret < 0)
    {
        LOGINFO("epoll ctrl failed type : %s",strerror(errno));
        exit(1);
    }
    return 0;
}

int epoll_accept(int epfd,int fd)
{
    struct sockaddr_in cliaddr;
    int len = sizeof(struct sockaddr_in);
    int cli = accept(fd,(struct sockaddr*)&cliaddr,(socklen_t*)&len);
    if(cli == -1)
    {

        LOGINFO("accept failed %s",strerror(errno));
        return cli;
    }
    LOGINFO("Connection from %s : %d",inet_ntoa(cliaddr.sin_addr),htons(cliaddr.sin_port));
    epoll_ctrl(epfd,EPOLL_CTL_ADD,cli);   
    return 0;
}

void do_process_client(int clifd,char *msg)
{
    parse_host(msg);
    int fd = create_connect();
    if(fd < 0)
    {
        LOGINFO("create_connect failed");
        return;
    }
    parse_msg(msg);

    LOGINFO("==============================  Send Message ==============================");
    printf("%s",msg);

    send(fd,msg,strlen(msg),0);

    char buf[4096] = {0};
    recv(fd,buf,sizeof(buf),0);
    LOGINFO("==============================  Recv Message ==============================");
    printf("%s",buf);

    send(clifd,buf,strlen(buf),0);
    LOGINFO("==============================  Send Message ==============================");
    printf("%s",buf);

    memset(buf,0,sizeof buf);
    recv(clifd,buf,sizeof(buf),0);
    LOGINFO("==============================  Recv Message ==============================");
    printf("%s",buf);

    send(fd,buf,strlen(buf),0);
    LOGINFO("==============================  Send Message ==============================");
    printf("%s",buf);
}


void epoll_read(int epfd,int fd)
{
    char buf[4096] = {0};
    int length = read(fd, buf, sizeof buf);
    if (length < 0)
    {
        LOGINFO("read failed");
    }
    else if (length == 0)
    {
        LOGINFO("client over");
        epoll_ctrl(epfd,EPOLL_CTL_DEL,fd);
    }
    else
    {
        /*LOGINFO("%s",buf);*/
        do_process_client(fd,buf);
        epoll_ctrl(epfd,EPOLL_CTL_ADD,fd);
    }
}


void epoll_loop(int listen_fd)
{
    int epfd = epoll_create(MAX_EVENTS);
    if (epfd < 0)
    {
        LOGINFO("create epoll failed");
        exit(1);
    }

    struct epoll_event events[MAX_EVENTS];

    epoll_ctrl(epfd,EPOLL_CTL_ADD,listen_fd);

    int nfds = 0;
    while(1)
    {
        nfds = epoll_wait(epfd,events,MAX_EVENTS,-1);
        if (nfds < 0)
        {
            LOGINFO("epoll_wait failed");
            exit(1);
        }

        for(int i = 0; i < nfds; ++i)
        {
            if (events[i].data.fd == listen_fd)
            {
                epoll_accept(epfd,events[i].data.fd);
            }
            else if(events[i].events == EPOLLIN)
            {
                epoll_read(epfd,events[i].data.fd);
            }
            else if(events[i].events == EPOLLOUT)
            {
                LOGINFO("write event\n");
            }
        }
    }
    close(epfd);
}

int prase_arg(int argc,char**argv)
{   
    if (argc < 2)
    {
        // port error
        LOGINFO("please enter port");
        exit(1);
    }
    int port = atoi(argv[1]);
    if (port > 65535 || port < 1024)
    {
        LOGINFO("prot failed");
        exit(1);
    }
    return port;
}

int main(int argc, char *argv[])
{
    int fd;
    int options = 1;
    int ret     = 0;
    int port    = prase_arg(argc,argv);

    fd = socket(AF_INET,SOCK_STREAM,0);
    if (fd < 0)
    {
        LOGINFO("socket failed");
        exit(1);
    }
    ret = setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&options,sizeof options);
    if(ret)
    {
        LOGINFO("setsockopt failed");
        exit(1);
    }
    struct sockaddr_in self;
    self.sin_family = AF_INET;
    self.sin_addr.s_addr = htonl(INADDR_ANY);
    self.sin_port = htons(port);
    ret = bind(fd,(const struct sockaddr*)&self,sizeof self);
    if (ret)
    {
        LOGINFO("bind failed");
        exit(1);
    }
    ret = listen(fd,20);
    if (ret)
    {
        LOGINFO("listen failed");
        exit(1);
    }
    epoll_loop(fd);
    close(fd);
    return 0;
}
