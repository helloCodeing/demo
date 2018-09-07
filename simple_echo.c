#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

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
    LOGINFO("host name   : %s",host_name);
}



void parse_msg(char*msg)
{
    char *p = msg;
    char *end = strchr(p,'\0'); 
    char *phttp = strstr(p,"http://");
    char *purl = phttp + 7;
    char *arg = strchr(purl,'/');
    int hlen = phttp - p;
    printf("hlen : %d\n",hlen);
    int len = end - arg;
    memcpy(phttp,arg,len);
    p[len + hlen] = '\0';
    LOGINFO("arg         : %s",p);
}

void parse_msg_one(char*msg)
{
    char buf[1024] = {0};
    char *p = msg;

    p = strstr(p,"\r\n");
    int len = p - msg;

    strncpy(buf,msg,len);
    LOGINFO("buf   : %s",buf);

    char *phttp = strstr(buf,"http://");
    LOGINFO("http        : %s",phttp);

    char *purl = phttp + 7;
    LOGINFO("url         : %s",purl);

    char *arg = strchr(purl,'/');
    LOGINFO("arg         : %s",arg);

    char *htype = strstr(buf,"HTTP/");
    LOGINFO("http type   : %s",htype);
}

int create_connection() {
    struct sockaddr_in server_addr;
    struct hostent *server;
    int sock;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    if ((server = gethostbyname(host_name)) == NULL) {
        return -1;
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    server_addr.sin_port = htons(RE_PORT);

    if (connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        return -1;
    }

    return sock;
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

void do_process_client(char *msg)
{     
    parse_host(msg);
    int fd = create_connect();
    /*int fd = create_connection();*/
    if(fd < 0)
    {
        LOGINFO("create_connect failed");
        return;
    }
    parse_msg(msg);
    LOGINFO("--> %s",msg);
    send(fd,msg,strlen(msg),0);
    char buf[1024] = {0};
    recv(fd,buf,sizeof(buf),0);
    printf("recv : %s\n",buf);
}

int main(int argc,char**argv)
{
    if (argc < 2)
    {
        printf("please enter port\n");
        return -1;
    }
    int port = atoi(argv[1]);
    if (port > 65535 && port < 1024)
    {
        printf("invalid port\n");
        return -1;
    }
    printf("listen port is : %d\n",port);

    int fd;
    fd = socket(AF_INET,SOCK_STREAM,0);
    if (fd < 0)
    {
        printf("socket create failed\n");
        return fd;
    }
    struct sockaddr_in addr;
    int ret;
    memset(&addr,0,sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port); 
    int options = 1;
    ret = setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&options,sizeof options);
    if (ret < 0)
    {
        printf("setsockopt failed\n");
        return ret;
    }

    ret = bind(fd,(const struct sockaddr*)&addr,sizeof(struct sockaddr_in));
    if(ret)
    {
        printf("bind failed\n");
        return ret;
    }
    ret = listen(fd,110);
    if (ret)
    {
        printf("listen failed\n");
        return ret;
    }
    struct sockaddr_in cliaddr;
    socklen_t len;

    int clifd = accept(fd,(struct sockaddr*)&cliaddr,&len);
    if (clifd < 0)
    {
        printf("accept failed\n");
        return -1;
    }
    
    char *ip = inet_ntoa(cliaddr.sin_addr);
    int cliport = ntohs(cliaddr.sin_port);
    printf("%s : %d  connected\n",ip,cliport);
    char buf[1024]={0};
    while(1)
    {
        memset(buf,0,sizeof buf);
        int length = read(clifd,buf,sizeof buf);
        if (length < 0)
        {
            printf("read failed\n");
            break;
        }
        else if (length == 0)
        {
            printf("client over\n");
            break;
        }
        
        printf("recv message is : %s\n",buf);
        /*write_msg(clifd,buf);       */
        do_process_client(buf);
    }

    close(fd);
    close(clifd);
    return 0;
}
