#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>


#define SERVER_PORT 6996

int fexists(char *fileName);

int main() {
  int                 serverSocket;
  struct sockaddr_in  serverAddr, clientAddr;
  int                 status, addrSize, bytesReceived;
  fd_set              readfds, writefds;
  char                buffer[256];
  char               response [100];
  char                fileBuffer[256];
  char temp[89];
  int counter = 0;
  char               type1[256];
  char*               fileName; 
  int                 rc; 
  FILE* fd;             
  
  //Ask the user for the name of the file to search though
  printf("Please enter the name of the pokemon file or press q to quit: ");
	while (1) {
		
		scanf("%ms",&fileName);		/* The %ms format will allocate the memory dynamically on the heap */
		//if user inputs q or Q program quits
		if (*fileName == 113 || *fileName == 81) {
			free(fileName);
			printf("Server: Shutting down.\n");
			close(serverSocket);  // Don't forget to close the socket !
			return 0;			
		} else {
			rc = fexists(fileName);
			if (rc == 1) {
				
				
				break;
			}
		}
	} 
	       

 
  // Create the server socket
  serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (serverSocket < 0) {
    printf("*** SERVER ERROR: Could not open socket.\n");
    exit(-1);
  }

  // Setup the server address
  memset(&serverAddr, 0, sizeof(serverAddr)); // zeros the struct
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddr.sin_port = htons((unsigned short) SERVER_PORT);

  // Bind the server socket
  status = bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
  if (status < 0) {
    printf("*** SERVER ERROR: Could not bind socket.\n");
    exit(-1);
  }
  
  
	


  // Wait for clients now
  while (1) {
    FD_ZERO(&readfds);
    FD_SET(serverSocket, &readfds);
    FD_ZERO(&writefds);
    FD_SET(serverSocket, &writefds);
    status = select(FD_SETSIZE, &readfds, &writefds, NULL, NULL);
    if (status == 0) {
      // Timeout occurred, no client ready
    }
    else if (status < 0) {
      printf("*** SERVER ERROR: Could not select socket.\n");
      exit(-1);
    }
    else {
      addrSize = sizeof(clientAddr);
      bytesReceived = recvfrom(serverSocket, buffer, sizeof(buffer),
                               0, (struct sockaddr *) &clientAddr, &addrSize);
                           
      if (bytesReceived > 0) {
		    buffer[bytesReceived] = '\0';
		    printf("SERVER: Received client request: %s\n", buffer);
		    strcpy(type1,buffer);
			
		  	
			int num_lines = 0;

	  		char line[256];
	  		char temp[256];
	  	
	  		fd = fopen(fileName, "r");
	  		fgets(temp, 89, fd);
	
		//Starts parsing though the CSV file
		fgets(line, 256, fd);	 
		//Until it reaches the end of the file, Store in the Pokemon full data, their type1 and output which will be zero 
		//And apply it to a linked list
		while(!feof(fd)){
			//printf("%s", line);
			strcpy(temp,line);
			char *token = strtok(temp, ",");
			
			while (token != NULL ) {
				counter++;
				if (counter == 3){
					counter = 0;
					int check = strcmp(token, type1);
					if (check == 0){
						//printf("SERVER: SENDING \"%s\" TO CLIENT", line);
						strcpy(response, line); 
		  				sendto(serverSocket, response, strlen(response), 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr));
					}
					break;
				}
				token = strtok(NULL,",");
			}		
			
			fgets(line, 256, fd);
		}

		    //close the file and the the client that we are done
		  	fclose(fd);
		    strcpy(response,"");
		 	strcpy(response, "done"); 
		  	sendto(serverSocket, response, strlen(response), 0, (struct sockaddr *) &clientAddr, sizeof(clientAddr));	
      }
      
     

      
      // If the client said to stop, then I'll stop myself
      if (strcmp(buffer, "stop") == 0)
	break;
    }
  } 

  // Don't forget to close the sockets!
  free(fileName);
  close(serverSocket);
  printf("SERVER: Shutting down.\n");
}

//Determine if the file exists or not
//Returns 0 if the file doesn't exist, Returns 1 if the file does exist
int fexists(char *fileName){
	FILE *fp = NULL;
    int rc = 0;

	// open the file
    fp = fopen(fileName, "r");

	// determine the return code
	if (!fp) {
		printf("Pokemon File is not found. Please enter the name of the file again or press q to quit: ");
		rc = 0;
	} 
	else {
		fclose (fp);
		rc = 1;
	}
	return(rc);
}
