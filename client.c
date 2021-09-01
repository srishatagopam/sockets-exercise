#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>

#define MAX 65527

int main(int argc, char *argv[])
{
    int s_udp, s_tcp;
    struct protoent *protoEnt_udp;
    struct protoent *protoEnt_tcp;
    struct sockaddr_in sin;
    struct hostent *host;
    char buf[MAX];
    ssize_t nbyte;
    int port;
    char protocol[5];

    if(argc != 4)
    {
        perror("error: not enough arguments");
        exit(-1);
    }

    port = atoi(argv[2]);
    strcpy(protocol, argv[3]);

    if ((protoEnt_udp = getprotobyname("udp")) == 0)
    {
        perror("error: getprotobyname(udp)");
        exit(-1);
    }
    if ((protoEnt_tcp = getprotobyname("tcp")) == 0)
    {
        perror("error: getprotobyname(tcp)");
        exit(-1);
    }

    //Populate sin
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons((uint16_t)port);
    
    //Populate host
    if(host = gethostbyname(argv[1]))
    {
        bcopy(host->h_addr, (char *)&sin.sin_addr, host->h_length);
    }
    else if( (sin.sin_addr.s_addr = inet_addr(argv[1])) == -1)
    {
        perror("error: gethostbyname");
    }

    /********************************************************************************/

    //UDP CODE
    if(!strcmp(protocol, "udp"))
    {
        //SOCKET -- UDP
        s_udp = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (s_udp < 0)
        {
            perror("error: udp socket");
            exit(1);
        }

        //CONNECT -- UDP
        if(connect(s_udp, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        {
            perror("can't connect to port");
            exit(1);
        }
        
        //READ/WRITE -- UDP
        strcpy(buf, "*");
        
        nbyte = write(s_udp, buf, strlen(buf));
        if(nbyte < 0)
        {
            perror("write failed udp");
        }

        nbyte = read(s_udp, buf, sizeof(buf));
        if(nbyte < 0)
        {
            perror("read failed udp");
        }
        
        puts(buf);
    }

    /********************************************************************************/
    
    //TCP CODE
    if(!strcmp(protocol, "tcp"))
    {

        //SOCKET -- TCP
        s_tcp = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s_tcp < 0)
        {
            perror("error: tcp socket");
            exit(1);
        }

        //CONNECT -- TCP
        if(connect(s_tcp, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        {
            perror("can't connect to port");
            exit(1);
        }

        //READ/WRITE -- TCP
        strcpy(buf, "*");
        nbyte = write(s_tcp, buf, strlen(buf));
        if(nbyte < 0)
        {
            perror("write failed tcp");
        }

        nbyte = read(s_tcp, buf, sizeof(buf));
        if(nbyte < 0)
        {
            perror("read failed tcp");
        }

        puts(buf);
    }
    /********************************************************************************/

    //CLOSE
    close(s_udp);
    close(s_tcp);
    return 0;
}