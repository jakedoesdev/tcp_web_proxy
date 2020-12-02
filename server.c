/* 
Author: Jacob Everett (jae0204)
Description: TCP web proxy server that receives request from client, searches list.txt for a cache entry, sends the contents of the cache to the client if
found and sends a GET request to the origin server if not. 
For new entries:  Creates entry in list.txt, saves response in new cache, and forwards response from origin to client.

Usage: ./pyserver <portnumber>

I meant to put everything in functions, but it got out of hand as I ran out of time.
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char **argv)
{
  char recvMsg[32768];    //holds messages read from client or origin
  char sendMsg[32768];   //holds messages for writing
  char getReq[1024];     //GET request for origin server
  char clientURL[1024];
  char timeStr[25];    //holds string representing time url was retrieved
  char listEntry[1024];
  int listenfd, connfd, sockfd, n, portNum;
  struct sockaddr_in servaddr;  //socket address struct for this server
  struct sockaddr_in urladdr;   //socket address struct for GET request origin server
  struct hostent *originHost;   //stores info retrieved from DNS lookup (gethostbyname())
  struct in_addr hostaddr;      //
  struct addrinfo hints;
  struct addrinfo *addrResult, *rp;

  //time variables
  struct tm tmStruct = {0};
  time_t currentTime;   
  char formatTime[20];

  //file variables
  FILE *fp;  //list file
  FILE *cfp;  //cache file
  int m, fnd, k = 0;
  char *buf = NULL;
  char *cBuf = NULL;
  size_t bufSize = 0;
  char urlMatch[1024];
  char cacheName[1024];


  if (argc != 2)
	{
		fprintf(stderr, "Usage: ./pyserver <portnumber>\n");
		exit(EXIT_FAILURE);
	}

  //Store user-entered port number
  portNum = atoi(argv[1]);
 
  //AF_INET - IPv4 IP , Type of socket, protocol
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htons(INADDR_ANY);
  servaddr.sin_port = htons(portNum);

  //Allow socket reuse
	int on = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
 
  //Bind struct to socket
	bind(listenfd,  (struct sockaddr *) &servaddr, sizeof(servaddr));

	//Listen for connections
	listen(listenfd, 10);

  //Accept connection
  connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);

  while(1)
  {
    fnd = 0;
    //if client sent URL...
    if (n = read(connfd, recvMsg, sizeof(recvMsg)) > 0)
    {
      //open list.txt to start checking for url
      fp = fopen("list.txt", "r");
      m = getline(&buf,&bufSize, fp);

      //copy url into clienturl for later (hacky last minute fix, I know this is dumb :( )
      strcpy(clientURL, recvMsg);
      
      //while this line has at least 1 character on it and match hasn't been found...
      while (m > 0 && fnd != 1)
      {
        //printf("m: %d\n", m);
        //printf("Current line: %s\n", buf);
        
        //if a match for the url is found on this line...
        if (strstr(buf, recvMsg))
        {
          //printf("Found match on: %s\n", buf);
          
          //set sentinel value to 1 and parse the line
          fnd = 1;
          for (int i=0; i<strlen(buf); i++)
          {
              //if this is the char between url and the date/time...
              if (buf[i] == ' ')
              {
                //store the url name in urlMatch
                strncpy(urlMatch, buf, i);
                urlMatch[i] = '\0';
                //printf("Match is: %s\n", urlMatch);

                //store the cache file name in cacheName
                strncpy(cacheName, buf+(i+1), 15);
                cacheName[i] = '\0';
                //printf("Cache file name is: %s\n", cacheName);

                //close the file and break out of for loop
                fclose(fp);
                break;
              }
          }
          //if a file named cacheName exists...
          if ((cfp = fopen(cacheName, "r")) != NULL)
          {
            //printf("Found cache file match on: %s\n", cacheName);

            //get each line and concat it to sendMsg
            k = getline(&buf,&bufSize, cfp);
            while (k > 0)
            {
              strcat(sendMsg, buf);
              k = getline(&buf,&bufSize, cfp);
              //printf("Sendmsg: %s\n", sendMsg);
              //printf("Buf: %s\n", buf);
            }
            //close cache file, set sentinel value to 1, write contents to client
            fclose(cfp);
            fnd = 1;
            write(connfd, sendMsg, strlen(sendMsg));
            bzero(sendMsg, 32768);
            bzero(cacheName, 1024);
            bzero(urlMatch, 1024);
          }
        }
        m = getline(&buf,&bufSize, fp);
      } //end line read while

      //printf("No cache file match found, contacting origin...\n");
      
      //no cache file found, need to contact origin server...
      //store data returned by DNS lookup
      originHost = gethostbyname(recvMsg);

      //if url can be resolved by gethostbyname()...
      if (originHost != NULL && fnd != 1)
      {
        //get first IP from origin
        hostaddr.s_addr = *(long int *) originHost->h_addr_list[0];

        //set up socket to connect to origin http server
        sockfd=socket(AF_INET, SOCK_STREAM, 0);
        bzero(&urladdr,sizeof(urladdr));
        urladdr.sin_family=AF_INET;
        urladdr.sin_port=htons(80);
        inet_pton(AF_INET,inet_ntoa(hostaddr),&(urladdr.sin_addr));

        //Connect to server
        connect(sockfd,(struct sockaddr *)&urladdr,sizeof(urladdr));

        //create and write get request using official hostname
        sprintf(getReq, "GET / HTTP/1.1\r\nHost: %s\r\n\r\n", originHost->h_name);
        write(sockfd, getReq, strlen(getReq));
        bzero(getReq, 1024);

        //if the server sends a reply...
        if (n = read(sockfd, recvMsg, sizeof(recvMsg)) > 0) 
        {

          //if 200 OK...
          if (strstr(recvMsg, "HTTP/1.1 200 OK"))
          {
            //printf("200 OK received\n");

            //store current time/date in currentTime
            time(&currentTime);

            //convert from GMT to local
            tmStruct = *localtime(&currentTime);

            //pull data from time struct to make YYYYMMDDhhmmss string
            strftime(formatTime, sizeof(formatTime), "%Y%m%d%H%M%S", &tmStruct);
            
            //printf("formatted time: %s", formatTime);
            //can't get urlMatch here, wasn't ever populated if we're here
            //printf("clientURL: %s\n", clientURL);
    
            //store recvMsg in file with formatTime as name
            fp = fopen(formatTime , "w");
            fprintf(fp, "%s", recvMsg);
            fclose(fp);

            //update list.txt
            //appends endlessly, need to add line count to start overwriting at 5 items
            fp = fopen("list.txt", "a+");

            //format line and print it to list.txt
            sprintf(listEntry, "%s %s", clientURL, formatTime);
            fprintf(fp, "\n%s", listEntry);
            fclose(fp);

            //write received data to client
            write(connfd, recvMsg, strlen(recvMsg));
            bzero(recvMsg, 32768);
          }
          //else anything but 200 OK...
          else
          {
            //printf("Not 200 OK\n");

            //write server reply to client
            write(connfd, recvMsg, strlen(recvMsg));
            bzero(recvMsg, 32768);
          }
        }
      }
    }
    //get next line
    m = getline(&buf,&bufSize, fp);
  }//end server while
  close(connfd);
  return 0;
}