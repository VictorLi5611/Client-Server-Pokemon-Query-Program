#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6996

#define IS_FIRST_ENTRY 'y'


//holds data for each pokemon
typedef struct PokemonData
{
	
	char info[100];
	

} EachPoke;

// Implements linked list data structure
typedef struct LinkedListNodeType
{
	EachPoke* pokeItems;
	void* next;
	
	
} ListNode;

typedef struct LinkedListNodeTypeForFileNames
{
	char name[100];
	struct LinkedListNodeTypeForFileNames* next;
}FileNode;

int getInput();
int add(ListNode* items, EachPoke* pokemon);
int findTailNode(ListNode** tailNode, ListNode* pokemon);
int fexists(char *fileName);
int removeTailNode(ListNode* items, int numNodes);
void removeTailNodeFiles(FileNode* items);
void push(struct LinkedListNodeTypeForFileNames** head, char* data);
void printTransactions(FileNode* trans);



int main() {
  int                 clientSocket, addrSize, bytesReceived;
  struct sockaddr_in  clientAddr;

  char                inStr[80];    // stores user input from keyboard
  char                buffer[256];   // stores sent and received data
  int                 command;
  int                 numFiles = 0;
  int                 numPokes = 0;
  char                firstEntry = 'y';
  char                firstEntryFiles ='y';
  int                 numQueries = 0;

  // Create socket
  clientSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (clientSocket < 0) {
    printf("*** CLIENT ERROR: Could open socket.\n");
    exit(-1);
  }

  // Setup address 
  memset(&clientAddr, 0, sizeof(clientAddr));
  clientAddr.sin_family = AF_INET;
  clientAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
  clientAddr.sin_port = htons((unsigned short) SERVER_PORT);
  
  ListNode* pokeData = malloc(sizeof(ListNode));
	pokeData->pokeItems = NULL;
	pokeData->next = NULL;
	
	FileNode* fileData = malloc(sizeof(FileNode));
	fileData->next = NULL;


  // Go into loop to commuincate with server now
  while (1) {
    // Get a command from the user
    
    command = getInput();
    //Search
    if (command == 1){
		printf("\nWhat type of pokemon do you want to search:  ");
		scanf("%s", inStr); 
		numQueries++;
		
		 // Send buffer string to server
    	strcpy(buffer, inStr);
    	//printf("CLIENT: Sending \"%s\" to server.\n", buffer);
    	sendto(clientSocket, buffer, strlen(buffer), 0,
           	(struct sockaddr *) &clientAddr, sizeof(clientAddr));
           	
       	while (strcmp(buffer,"done") != 0 ){
		   	// Get response from server
					addrSize = sizeof(clientAddr);
	   			bytesReceived = recvfrom(clientSocket, buffer, sizeof(buffer), 0,
				    (struct sockaddr *) &clientAddr, &addrSize);
				    
					buffer[bytesReceived] = 0; // put a 0 at the end so we can display the string
					//printf("CLIENT: RECIVING \"%s\" from Server.\n" , buffer);
			
					if (strcmp(buffer,"done") == 0){
						break;
					}
					EachPoke* data = malloc(sizeof(EachPoke));
					strcpy(data->info, buffer);
					if (firstEntry == IS_FIRST_ENTRY)
					{
						pokeData->pokeItems = data;
						firstEntry = 'n';
					}else{
						numPokes = add(pokeData,data);
					}
					strcpy(buffer,"");	
       		}	
    }
    //save
    if (command == 2){
    	
    	FILE *readOut;
			char* outfile;
			
			while(1){

				printf("\nWhat should the name of the file be?: ");
				scanf("%ms" , &outfile);
				if(( readOut = fopen(outfile, "w+")) == NULL) {
					printf("ERROR: can't create file. Please enter a vaild name");
				} else{
					numFiles++;
					if (firstEntryFiles == IS_FIRST_ENTRY)
					{
						strcpy(fileData->name, outfile);
						firstEntryFiles = 'n';
					}else{
						push(&fileData,outfile);
						}
						free(outfile);
						break;
					
					}
				}	
			//save the name of the file into a linked list
			ListNode* currentNode = pokeData;
			while(currentNode != NULL){
				 
				
				fputs(currentNode->pokeItems->info, readOut);
				currentNode = currentNode->next;
			}
			fclose(readOut);				
    } 
        
    if (command == 3){
    //tell the server to shut down
    strcpy(buffer,"stop");
    //printf("CLIENT: Sending \"%s\" to server.\n", buffer);
    	sendto(clientSocket, buffer, strlen(buffer), 0,
           	(struct sockaddr *) &clientAddr, sizeof(clientAddr));
           	
       //print file names
       FileNode* currentNode = fileData;
	printf("Here are the number of queries that were success: %d\n", numQueries);

	//goes though the entire linked list and print out content
	
	printTransactions(fileData); 
	
	int remainingNodes = numPokes;
			while(remainingNodes > 1)
			{
				remainingNodes = removeTailNode(pokeData,remainingNodes);
			}
			
			removeTailNodeFiles(fileData);
			free(pokeData->pokeItems);
			free(pokeData);     
      
      break;
      }
  } 

  close(clientSocket);  // Don't forget to close the socket !
  printf("CLIENT: Shutting down.\n");
}

void push(struct LinkedListNodeTypeForFileNames** head, char* data){
	
	//allocate node
	FileNode* newNode =  malloc(sizeof(FileNode));
	
	//put in data
	strcpy(newNode->name,data);
	
	//make new node as head
	newNode->next = (*head);
	
	(*head) = newNode;
}


int getInput(){
	char* user = NULL;
	int output;
	

	printf("CLIENT:\n");
	printf("a. Type search (press 1)\n");
	printf("b. Save results (press 2)\n");
	printf("c. Exit the Program (press 3)\n");
	
	while (1) {
		
		scanf("%ms",&user);		/* The %ms format will allocate the memory dynamically on the heap */
		
		if (*user == 49|| *user ==50 || *user == 51){
			break;
		}else{
			printf("Not valid input, please try again\n\n");
			
		}
	}
	
	output = atoi(user);
	free(user);
	return output;
}

//Function: add()
//This function add each transaction to the linked list which holds all of the transaction
int add(ListNode* items, EachPoke* transaction)
{
	ListNode* tailNode = NULL;
	int numNodes = findTailNode(&tailNode, items);
	
	//allocate new memory and add the struct
	ListNode* newNode = malloc(sizeof(ListNode));
	newNode->pokeItems = transaction;
	newNode->next = NULL;
	
	tailNode->next = newNode;
	numNodes++;
	
	return numNodes;
	
	
}





//Function: findTailNode()
// Located the tail node of the list
//Return the total number of nodes in the list
int findTailNode(ListNode** tailNode, ListNode* transaction)
{
	int totalNodes = 0;
	//Find the tail of the linked list
	ListNode* tail = NULL;
	ListNode* currentNode = transaction;
	while(tail == NULL){
		if (currentNode->next == NULL)
		{
			tail = currentNode;
		}else{
			currentNode = (ListNode*)currentNode->next;
		}
		totalNodes++;
	}
	
	*tailNode = tail;
	return totalNodes;
}



int removeTailNode(ListNode* items, int numNodes)
{
	ListNode* tailNode = NULL;
	ListNode* newTail = items;
	for(int i=2;i<numNodes;i++)
	{
		newTail = (ListNode*)newTail->next;
	}
	tailNode = (ListNode*)newTail->next;
	
	free(tailNode->pokeItems);
	free(newTail->next);
	numNodes--;
	
	return numNodes;
	
		
}

void removeTailNodeFiles(FileNode* items)
{
	void *delete;
	while(items)
	{
		delete = items;
		items = items->next;
		free(delete);
	}
		
}

//Function: printTransactions()
//Prints all of the transaction, number of transactions, the average amount
void printTransactions(FileNode* trans)
{
	FileNode* currentNode = trans;
	printf("\nHere are all of the files:\n");
	//goes though the entire linked list and print out content
	//Add to the total amount to calculate the average
	while(currentNode != NULL)
	{
		
		printf("%s\n", currentNode->name);
		
		currentNode = currentNode->next;
		
	}
	
	
	
}


