#include <stdio.h>       // for printf() and fprintf()
#include <sys/socket.h>  // for socket(), bind(), and connect()
#include <arpa/inet.h>   // for sockaddr_in and inet_ntoa()
#include <stdlib.h>      // for atoi() and exit()
#include <string.h>      // for memset()
#include <unistd.h>      // for close()
#include <netdb.h>       // for hostent
#include <sys/types.h>
#include <stdio.h>

#include <sys/time.h>
#include <libgen.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

//#define BUFFER_SIZE 1310720  /* Size of receive buffer */
#define BUFFER_SIZE 1024

// function prototypes*
int hostname_to_ip(char *  , char *);
void DieWithError(char *errorMessage);  // Error handling function
void printtime(struct timeval tB, struct timeval tA); // for -p print option

struct timeval tvalBefore, tvalAfter; //create time structs for beginning and end of program

void printtime(struct timeval tB, struct timeval tA){
  printf("Time in microseconds: %ld microseconds\n",
            ((tvalAfter.tv_sec - tvalBefore.tv_sec)*1000000L
           +tvalAfter.tv_usec) - tvalBefore.tv_usec
        );
  printf("Time in milliseconds: %ld milliseconds\n",
            (((tvalAfter.tv_sec - tvalBefore.tv_sec)*1000000L
           +tvalAfter.tv_usec) - tvalBefore.tv_usec)/1000
        );
}

// Usage: ./http_client [-options] server_url port_number
int main(int argc, char* argv[])
{
  //To make the output easier to read
  printf("******************************************* \n");
  char* server_url; // URL destination
  int port_number; //portnumber
  int print_flag = 0; //Print option, intialized to 0, 1 if flag checked

  // if there is no option flag
  if(argc == 3){
    //check if proper values
    server_url = argv[1];
    port_number = atoi(argv[2]);

  else if(argc == 4){
    printf("Option: %s\n", argv[1]);
    //check if it is a proper value
    if(strcmp(argv[1], "-p") == 0)
      print_flag = 1;
    else {
      printf("Not a proper option flag\n");
      return 1;
    }
    server_url = argv[2];
    port_number = atoi(argv[3]);
  }
  // if the command line arguents are not between 3 and 4
  else {
    fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]);
    exit(1);
  }

  printf("Server_URL: %s, Port_Number: %ld\n", server_url, port_number);

  //Get the host IP address
  //char *hostname = argv[1];
  char ip[100];

  //get the ip address form server_url
  hostname_to_ip(server_url , ip);

  printf("Getting the IP of host \n");
  printf("%s resolved to %s\n" , server_url , ip);

  int sock;                          // Socket descriptor
  struct sockaddr_in echoServAddr;   // Echo server address
  unsigned short echoServPort;       // Echo server port
  char *servIP;                      // Server IP address (dotted quad)
  char *echoString;                  // String to send to echo server
  echoString = (char *) malloc(400);
  char echoBuffer[BUFFER_SIZE];      // Buffer for echo string
  unsigned int echoStringLen;        // Length of string to echo
  int bytesRcvd, totalBytesRcvd;     // Bytes read in single recv() and total bytes read

  //use the given port
  echoServPort = port_number;

  //Get the Host and the Page
  char arg[500];

  strcpy(arg, server_url);
  char * pch;
  pch=strchr(arg,'/');

  char host[500];
  char pagepath[500];
  if(pch != NULL){
    strncpy(host, arg, pch-arg);
    printf("pch-arg: %d\n", pch-arg);
    host[pch-arg] = '\0';
    memcpy(pagepath, &arg[pch-arg], strlen(arg)); //copy rest of string
    pagepath[strlen(arg)+1] = '\0';
  } else {
    strcpy(host, arg);
    host[strlen(arg)] = '\0';
    strcpy(pagepath, "/\0"); //insert generic / for proper GET request
  }

  printf ("Host: %s\n",host);
  printf ("Page: %s\n",pagepath);
  printf ("Together: %s%s\n", host, pagepath);

  //Retreive host ip after we've separated page from host
  hostname_to_ip(host , ip);
  servIP = ip;

  char request[400];
  sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", pagepath, host);
  strcpy(echoString, request);
  //echoString = &request;

  //Change to passed URL for host
  printf("Get Request: \n");
  printf("%s\n", echoString);

  /* Create a reliable, stream socket using TCP    SOCK_NONBLOCK */
  if ((sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    printf("Failed to create socket\n");
    //DieWithError("socket() failed");
  printf("Created Socket \n");

  /* Construct the server address structure */
  memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
  echoServAddr.sin_family      = AF_INET;             /* Internet address family */
  echoServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
  echoServAddr.sin_port        = htons(echoServPort); /* Server port */
  //printf("Constructed server address \n");

  if(print_flag == 1){
    gettimeofday (&tvalBefore, NULL);
    //printf("Time at start of copy: %d.%d\n", tvalBefore.tv_sec, tvalBefore.tv_usec);
  }

  /* Establish the connection to the echo server */
  if (connect (sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
    printf("Failed to connect\n");
    //DieWithError("connect() failed");
  printf("Connection Established \n");

  echoStringLen = strlen(echoString);          /* Determine input length */

  /* Send the string to the server */
  if (send (sock, echoString, echoStringLen, 0) != echoStringLen)
    printf("Failed to send \n");
    //DieWithError("send() sent a different number of bytes than expected");
  printf("Sent Request \n");

  /* Receive the same string back from the server */
  totalBytesRcvd = 0;
  printf("Received: ");                /* Setup to print the echoed string */


  //get the name of the first part of the directory of the file
  //so we can name the downloaded pages unique(ish)ly

  //pch = strtok (pagepath,"/");
  //printf ("Name of File:  %s\n",pch);


  FILE *fp;

  int FoundFile = 0;
  //No header for ccc page, so skip truncating the header
  if(strstr (echoBuffer,"cccwork") == NULL){
    FoundFile = 1;
    fp = fopen("index.html", "r+"); //only need to read it in this case
  }
  else{
    //fp = fopen(strcat(pch, ".html"), "r+");
    fp = fopen("index.html", "w+");
  }


  while(recv(sock, echoBuffer, BUFFER_SIZE, 0) > 0){
    printf("%s", echoBuffer); //output full msg to the terminal
    //printf("Count: %d,  recv: %d\n", count, bytes);

      if(FoundFile == 0){
        char * pch2;
        pch2 = strstr (echoBuffer,"\r\n\r\n");
        //printf("%s", pch2);
        fputs(pch2, fp);
        FoundFile = 1;
        //printf("0\n");
      }
      else{
        //printf("1\n");
        fputs(echoBuffer, fp);
      }
      memset(echoBuffer, 0, sizeof(echoBuffer));

    }


  //echoBuffer[bytesRcvd] = '\0';

  printf("\n");    /* Print a final linefeed */
  close (sock);
  fclose(fp);


  if(print_flag == 1){
    gettimeofday (&tvalAfter, NULL);

    //******TIME*******
    /*Print the start and end time of copy as well as total time in microseconds
       Epoch time, meaning the time in seconds since midnight (0 hour)
        January 1, 1970 that have passed until now.
    */
    printf("TIME STAMPS (Epoch time) \n");
    printf("Time at start of copy: %d.%d\n", tvalBefore.tv_sec, tvalBefore.tv_usec);
    printf("Time at end of copy:   %d.%d\n", tvalAfter.tv_sec, tvalAfter.tv_usec);


    //Find the total time it took to copy based off the two time stamps
    printtime(tvalBefore, tvalAfter);
  }

  //To make the output easier to read
  printf("******************************************* \n");

  return 0;
}
//*********************************************************************


int hostname_to_ip(char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    //try to find the host ip
    if ( (he = gethostbyname( hostname ) ) == NULL) {
        herror("gethostbyname");
        return 1;
    }

    addr_list = (struct in_addr **) he->h_addr_list;

     //convert to proper format
    for(i = 0; addr_list[i] != NULL; i++)
    {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }
    return 1;
}




/** Copies one file to another using formatted I/O, one character at a time.
 @param infilename Name of input file
 @param outfilename Name of output file
 @return 0 if successful, 1 if error.
*/
