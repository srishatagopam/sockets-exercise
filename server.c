#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>

#define INADDR_ANY (ipaddr_t)0x00000000
#define MAX 65527
#define PORT 5018

int main()
{
    int s_udp, s_tcp;
    struct protoent *protoEnt_udp;
    struct protoent *protoEnt_tcp;
    struct sockaddr_in sin, cin;
    struct hostent *host;
    char buf[MAX];
    char msg[50];
    int acc;

    if ((protoEnt_udp = getprotobyname("udp")) == 0)
    {
        perror("error: getprotobyname(udp)\n");
        exit(-1);
    }
    if ((protoEnt_tcp = getprotobyname("tcp")) == 0)
    {
        perror("error: getprotobyname(tcp)\n");
        exit(-1);
    }

    //Populate sin
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons((uint16_t)PORT);
    sin.sin_addr.s_addr = INADDR_ANY;

    //Reset cin - used for UDP to connect to client
    bzero((char *)&cin, sizeof(cin));

    //Populate host - use your host name here
    char *hostname = "xxx";
    if(host = gethostbyname(hostname))
    {
        bcopy(host->h_addr, (char *)&sin.sin_addr, host->h_length);
    }
    else if( (sin.sin_addr.s_addr = inet_addr(hostname)) == -1)
    {
        perror("error: gethostbyname\n");
    }

    /********************************************************************************/
    
    //SOCKET
    s_udp = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    s_tcp = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s_udp < 0)
    {
        perror("error: udp socket\n");
        exit(1);
    }
    if (s_tcp < 0)
    {
        perror("error: tcp socket\n");
        exit(1);
    }

    /********************************************************************************/
    
    //BIND
    if(bind(s_udp, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        fprintf(stderr, "can't bind to udp port %d\n", PORT);
        exit(1);
    }
    if(bind(s_tcp, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        fprintf(stderr, "can't bind to tcp port %d\n", PORT);
        exit(1);
    }

    /********************************************************************************/
    
    //LISTEN -- ONLY FOR TCP
    if(listen(s_tcp, 5) < 0)
    {
        perror("error: listen\n");
        exit(1);
    }
    
    /********************************************************************************/
    
    fd_set rfds;
    FD_ZERO(&rfds);
    int max = (s_udp > s_tcp) ? s_udp : s_tcp;
    int nfds = max + 1;

    while(1)
    {
        FD_SET(s_udp, &rfds);
        FD_SET(s_tcp, &rfds);
        if( select(nfds, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0)
        {
            perror("select:\n");
        }

        time_t now;
        
        //SEND/RECV
        if(FD_ISSET(s_udp, &rfds))
        {
            //Use cin (client) to recv from and send to client.
            ssize_t fromlen = sizeof(cin);
            if(recvfrom(s_udp, buf, sizeof(buf), 0, (struct sockaddr *)&cin, &fromlen) < 0)
            {
                perror("recvfrom: udp\n");
            }

            time(&now);

            char *timeval = ctime(&now);
            if(sendto(s_udp, timeval, strlen(timeval), 0, (struct sockaddr *)&cin, fromlen) < 0)
            {
                perror("sendto: udp\n");
            }
        }
        if(FD_ISSET(s_tcp, &rfds))
        {
            //ACCEPT -- ONLY FOR TCP
            acc = accept(s_tcp, 0, 0);
            if(acc < 0)
            {
                perror("error: accept\n");
                exit(1);
            }

            ssize_t nbyte;
            nbyte = read(acc, buf, strlen(buf));

            time(&now);
            strcpy(buf, ctime(&now));
            write(acc, buf, strlen(buf));
        }

    close(acc);
    }

    /********************************************************************************/
    
    //CLOSE
    close(s_udp);
    close(s_tcp);

    return 0;
}
