#include <stdio.h>          /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>   /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>       /* for atoi() and exit() */
#include <string.h>       /* for memset() */
#include <unistd.h>      /* for close() */
#include <netdb.h>       //hostent
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

//#define RCVBUFSIZE 655360  /* Size of receive buffer */
#define RCVBUFSIZE 1024

#define MAXPENDING 8    /* Maximum outstanding connection requests */
void DieWithError(char *errorMessage);  /* Error handling function */
void HandleTCPClient(int clntSocket);   /* TCP client handling function */


// function prototype **MOVE TO .h file***
void DieWithError(char *errorMessage);  /* Error handling function */ 

void DieWithError(char *errorMessage){
  printf("%s\n", errorMessage);
  exit(1);
}


struct timeval tvalBefore, tvalAfter; //create time structs for beginning and
                                      //end of program

/* printtime Finds the difference in time that passed to copy a file
  *Changed format to long int (%ld), changed time calculation*
   @param tB timeval struct taken right before copying
   @param tA timeval struct taken right after copying
   @return totaltime

*/
void printtime(struct timeval tB, struct timeval tA){
  printf("Time in microseconds: %ld microseconds, ",
            ((tvalAfter.tv_sec - tvalBefore.tv_sec)*1000000L
           +tvalAfter.tv_usec) - tvalBefore.tv_usec
          ); 
  printf("Time in milliseconds: %ld milliseconds\n",
            (((tvalAfter.tv_sec - tvalBefore.tv_sec)*1000000L
           +tvalAfter.tv_usec) - tvalBefore.tv_usec)/1000
          ); 
}

//*********************************************************************
/** Main program: Start a server with an open port
    @param argc Number of command-line arguments (including program name).
    @param argv Array of pointers to character arays holding arguments.
    @return 0 if successful, 1 if fail.
    Usage: ./http_server port_number
*/
int main(int argc, char* argv[])
{
 
//****************Class notes**************************************
  int servSock;                    /*Socket descriptor for server */
  int clntSock;                    /* Socket descriptor for client */
  struct sockaddr_in echoServAddr; /* Local address */
  struct sockaddr_in echoClntAddr; /* Client address */
  unsigned short echoServPort;     /* Server port */
  unsigned int clntLen;            /* Length of client address data structure */ 
  
  int fd;                    /* file descriptor for file to send */
  char filename[PATH_MAX];   /* filename to send */
  struct stat stat_buf;      /* argument to fstat */
  off_t offset = 0;          /* file offset */
  int rc;                    /* holds return code of system calls */


  if (argc != 2)     /* Test for correct number of arguments */
  { 
    fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
    exit(1);   
  }
  echoServPort = atoi(argv[1]);  /* First arg:  local port */

     
  /* Create socket for incoming connections */
  if ((servSock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) 
    DieWithError("socket() failed");

  /* Construct local address structure */
  memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
  echoServAddr.sin_family = AF_INET;                /* Internet address family */
  echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
  echoServAddr.sin_port = htons(echoServPort);      /* Local port */
    
    /* Bind to the local address */
   if (bind (servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
     DieWithError("bind() failed");

    /* Mark the socket so it will listen for incoming connections */
    if (listen (servSock, MAXPENDING) < 0)
      DieWithError("listen() failed");

    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        clntLen = sizeof(echoClntAddr);        /* Wait for a client to connect */
        if ((clntSock = accept (servSock, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0)         
          DieWithError("accept() failed");

        printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

        /* clntSock is connected to a client! */
        //printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));
        HandleTCPClient(clntSock);
     }
     /* NOT REACHED */

//****************Class notes**************************************

  //To make the output easier to read
  printf("******************************************* \n");

  return 0;
}
//*********************************************************************


void HandleTCPClient(int clntSocket)
{
  int fd;                    /* file descriptor for file to send */
  char filename[PATH_MAX];   /* filename to send */
  struct stat stat_buf;      /* argument to fstat */
  off_t offset = 0;          /* file offset */
  int rc;                    /* holds return code of system calls */


  char echoBuffer[RCVBUFSIZE]; 
  int recvMsgSize;                    /* Size of received message */

  gettimeofday (&tvalBefore, NULL);
  
  /* Receive message from client */
  if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0)
    DieWithError("recv() failed");
  //printf("recvMsgSize: %d\n", recvMsgSize);


  //Trim echobuffer to file path
  const char s[2] = " /";
  char *token;
   
  /* get the first token */
  token = strtok(echoBuffer, s);
  token = strtok(NULL, s); //iterate to the next token
   
  strcpy(echoBuffer, token);
  //fprintf(stderr, "received request to send file %s\n", echoBuffer);
  // echoBuffer = token;

  /* null terminate and strip any \r and \n from filename */
  echoBuffer[recvMsgSize] = '\0';
  if (echoBuffer[strlen(echoBuffer)-1] == '\n'){
    echoBuffer[strlen(echoBuffer)-1] = '\0';
    //printf("1****");
  }
      
  if (echoBuffer[strlen(echoBuffer)-1] == '\r'){
    echoBuffer[strlen(echoBuffer)-1] = '\0';
    //printf("2****");
  }
  

  fprintf(stderr, "received request to send file: %s\n", echoBuffer);

  offset = 0;
  //Check to see if file exists, 200 if it does, 404 if not found
  fd = open(echoBuffer, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "unable to open '%s': %s\n", echoBuffer, strerror(errno));
    printf("404 Page not found\n");
    //char *reply = "HTTP/1.1 404 Page not found\n";
    int fd2 = open("404NotFound.html", O_RDONLY);
    if (fd2 == -1) 
      printf("Couldnt even open 404 page\n");
    //fstat(fd2, &stat_buf);
    offset = 0; 
    sendfile (clntSocket, fd2, &offset, 1024);
    //send(clntSocket, &fd2, sizeof(fd2), 0); //Otherwise need to char* and then fread in the characters
    close(clntSocket);
    exit(1);
  }
  printf("200 OK\n");

  /* get the size of the file to be sent */
  fstat(fd, &stat_buf);
  //printf("File size: %d\n", stat_buf.st_size);

  /* copy file using sendfile */
  offset = 0;
  //recvMsgSize = sendfile (clntSocket, fd, &offset, stat_buf.st_size);

  int count = 0;
  int sentMsgSize = 5;
  while(sentMsgSize = sendfile (clntSocket, fd, &offset, 1024) > 0){
      //printf("Count: %d, sentMsgSize: %d\n", count, sentMsgSize);
      count++;
      offset += sentMsgSize;
  }

  if (offset == -1) {
    fprintf(stderr, "error from sendfile: %s\n", strerror(errno));
    exit(1);
  }
           //   recvMsgSize,

  if (offset < stat_buf.st_size) {
    fprintf(stderr, "incomplete transfer from sendfile: %d of %d bytes\n",
            offset,
            (int)stat_buf.st_size);
    exit(1);
  }
  /* close descriptor for file that was sent */
  close(fd);

  /* close socket descriptor */
  close(clntSocket);
  
  

  //******TIME*******
  /*Print the start and end time of copy as well as total time in microseconds
    Epoch time, meaning the time in seconds since midnight (0 hour)
    January 1, 1970 that have passed until now.
  */
  gettimeofday (&tvalAfter, NULL);
  printf("TIME STAMPS (Epoch time) \n");
  printf("Time at start of copy: %d.%d, ", tvalBefore.tv_sec, tvalBefore.tv_usec);
  printf("Time at end of copy: %d.%d\n", tvalAfter.tv_sec, tvalAfter.tv_usec);

  //Find the total time it took to copy based off the two time stamps
  printtime(tvalBefore, tvalAfter);
  
}




