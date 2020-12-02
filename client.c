/* 
Author: Jacob Everett (jae0204)
Description:  TCP client that sends a url to a server and waits to read a response.
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
 
int main (int argc, char **argv)
{   
    char url[1024];
    char getReq[1024];
    int sockfd, n, portNum;
    int len = sizeof(struct sockaddr);
    char recvMsg[32768];
    char sendMsg[1024];
    struct sockaddr_in servaddr;

    if (argc != 2)
	{
		fprintf(stderr, "Usage: ./client <portnumber>\n");
		exit(EXIT_FAILURE);
	}

    /* AF_INET - IPv4 IP , Type of socket, protocol*/
    sockfd=socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr,sizeof(servaddr));
 
    //store port num from user-entered arg
    portNum = atoi(argv[1]);
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(portNum); // Server port number
 
    /* Convert IPv4 and IPv6 addresses from text to binary form */
    inet_pton(AF_INET,"129.120.151.94",&(servaddr.sin_addr));
 
    /* Connect to server */
    connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

    while (1)
    {
        printf("url: ");
        scanf("%s", url);

        //write url to server
        write(sockfd, url, strlen(url));
        bzero(url, 1024);

        //print server response
        if (n = read(sockfd, recvMsg, sizeof(recvMsg)) > 0)
        {
            printf("%s\n", recvMsg);
        }

        bzero(recvMsg, 32768);
    }
    close(sockfd);
    return 0;
}
