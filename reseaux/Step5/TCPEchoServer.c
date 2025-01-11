#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#include <netdb.h>      /* For Getinfo */

#define MAXPENDING 5    /* Maximum outstanding connection requests */

#include "LibSer.h"  	/* Error handling function */
#include "HandleTCPClient.h"   /* TCP client handling function */
#include "LibSerHV.h"
int main(int argc, char *argv[])
{
    int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
    struct sockaddr_in *echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned short echoServPort;     /* Server port */
    unsigned int clntLen;            /* Length of client address data structure */
    char *servIP;                    /* Server IP address (dotted quad) */
    char *hostName ;                  /* hostName Server */

    char host[NI_MAXHOST];
    char port[NI_MAXSERV];
    if (argc != 3)     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
        exit(1);
    }
    hostName = argv[1] ;
    echoServPort = atoi(argv[2]);  /* First arg:  local port */


    struct addrinfo* addr;
    int result = getaddrinfo(hostName, NULL, NULL, &addr); /* String to sockaddr_in */
    if (result != 0)
    {
        printf("Error from getaddrinfo: %s\n", gai_strerror(result));
        exit(0) ;
    }
    else
    {

       /* memset(&echoServAddr, 0, sizeof(echoServAddr));    Zero out structure */
                      
        echoServAddr  = (struct sockaddr_in*) addr->ai_addr; /* Pointer to sockaddr_in Local address IP */
        echoServAddr->sin_family = AF_INET; /* Interenet address family */
        servIP = inet_ntoa(echoServAddr->sin_addr) ;
        printf("getaddrinfo: %s\n", servIP);
        echoServAddr->sin_port = htons(echoServPort);      /* We add Local port */ 
      }

    /* Create socket for incoming connections */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");
    else
       printf("socket() Ok\n") ;
      
    /* Construct local address structure */
    // memset(echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr->sin_family = AF_INET;                /* Internet address family */
    echoServAddr->sin_addr.s_addr = inet_addr(servIP); /* Any incoming interface */
    echoServAddr->sin_port = htons(echoServPort);      /* Local port */

    /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *) echoServAddr, sizeof(struct sockaddr_in)) < 0)
        DieWithError("bind() failed");
    else
       printf("bind() Ok\n") ;    

    /* Mark the socket so it will listen for incoming connections */
    if (listen(servSock, MAXPENDING) < 0)
        DieWithError("listen() failed");
    else
       printf("listen() Ok\n") ;     

    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        clntLen = sizeof(echoClntAddr);

        /* Wait for a client to connect */
        if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, 
                               &clntLen)) < 0)
            DieWithError("accept() failed");
        else
           printf("accept() Ok\n") ;     


        /* clntSock is connected to a client! */
        getpeername(servSock,(struct sockaddr*)&echoClntAddr,&clntLen);
        getnameinfo((struct sockaddr*)&echoClntAddr,clntLen,
        host, NI_MAXHOST,
        port, NI_MAXSERV,
        NI_NUMERICSERV | NI_NUMERICHOST);
        printf("Client connecte --> Adresse IP: %s -- Port: %s\n",host,port);
        
        printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

        HandleTCPClient(clntSock);
    }
    /* NOT REACHED */
}
