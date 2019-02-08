#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<dirent.h>
#include<pthread.h>
#include<stdbool.h>
#include<semaphore.h>
#include<signal.h>
#include<errno.h>
#include<fcntl.h>
#include<sys/time.h>
#include"database.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t thread_id[5000];
sem_t mut;
char message[256];
int fd[10000];
int count = 0;
int clientnum = 0;
struct account* head = NULL;
void * handler(void*);
void * diaprint(void*);
void diag(void);
int sockfd = -1;
int newsockfd = -1;
//struct itimerval it_val;
void handle_sigint(int sig){
	
	int tmp = count;
	strcpy(message,"quit");
	int ptr = clientnum;
	while(ptr > 0){
		write(fd[ptr-1],message,strlen(message));
		close(fd[ptr]);
		ptr --;
	}
	struct itimerval value;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_usec = 0;
	value.it_interval = value.it_value;
	setitimer(ITIMER_REAL,&value,NULL);
	//close(newsockfd);
	close(sockfd);
	while(tmp>0){
		pthread_cancel(thread_id[tmp-1]);
		tmp--;	
	}
	tmp = count;
	while(tmp>0){
		pthread_join(thread_id[tmp-1],NULL);
		tmp--;
	}
	sem_wait(&mut);
	freestruct(head);
	sem_post(&mut);
	sem_destroy(&mut);
	exit(0);
}

int main(int argc, char *argv[]){
	signal(SIGINT,handle_sigint);
while(1){
	sem_init(&mut,0,1);
	//Declare initial vars
	//int sockfd = -1;	//file descriptor for our socket
	int portno = 55555;	//server port to connect to
	int n = -1;		//utility variable - for monitoring reading/writing from/to the socket
	char buffer[256];	//char array to store data going to and coming from the server
	struct sockaddr_in clientAddressInfo;
	struct sockaddr_in serverAddressInfo;	//Super-special secret C struct that holds address info 
	struct hostent *serverIPAddress;	//Super-special secret C struct that holds info about a mechin
	//if the user didn't enter enough arguments. complain and exit
	if(argc != 2){
		fprintf(stderr,"[S]usage %s hostname port\n", argv[0]);
		exit(0);
	}
	portno = atoi(argv[1]);
	/**If the user gave enough arguents, try to use them to get a port number and address**/
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==0){
		fprintf(stderr,"[S]socket failed\n");
		exit(0);
	}
	const void *so_linger;
	if(setsockopt(sockfd,SOL_SOCKET,SO_LINGER,&so_linger,sizeof(so_linger))!=0){
		fprintf(stderr,"[S]setsockopt failed\n");
		exit(0);
}
	/** We now have the port to build our server socket on .. time to set up the address struct**/

	//zero out the socket address info struct..always initialize!
	bzero((char*)&serverAddressInfo,sizeof(serverAddressInfo));
	
	//set the remote port ... translate from a 'normal' int to a super-special network-port-int'
	serverAddressInfo.sin_port=htons(portno);

	//set a flag to indicate the type of network address we'll be using
	serverAddressInfo.sin_family=AF_INET;

	//set a flag to indicate the type of network address we'll be willing to accept connections from
	
	serverAddressInfo.sin_addr.s_addr=INADDR_ANY;

	/**We have an address struct and a socket ... time to build up the server socket**/
	//bind the server socket to a specific local port, so the client has a target to connect to
	if(bind(sockfd,(struct sockaddr*) &serverAddressInfo, sizeof(serverAddressInfo))<0){
		fprintf(stderr,"[S]ERROR on binding\n");
		exit(0);
	}
	//ser up the server socket to listen for client connections
	if(listen(sockfd,20)<0){
		fprintf(stderr,"[S]Error in listen\n");
		exit(0);
	}

	//determine the size of a clientAddressInfo struct
	int clilen = sizeof(clientAddressInfo);
	//int newsockfd = 0;
	
	pthread_create(&thread_id[count],NULL,diaprint,NULL);
	//block until a client connects, when it does, create a client socket
	while(1){
		newsockfd=accept(sockfd,(struct sockaddr*)&clientAddressInfo, (socklen_t*)&clilen);

		if(newsockfd <0){
			fprintf(stderr,"[S]ERROR on accpet\n");
			break;
		}
		int* new_sock = (int*)malloc(1);
		*new_sock = newsockfd;	
		if(pthread_create(&thread_id[count],NULL,handler,(void*)new_sock)<0){
			fprintf(stderr,"[S]ERROR on create thread\n");
			break;
		}
		printf("[S]Accpetance of connection from client\n");
		fflush(stdout);
	//close(newsockfd);
	}
}
	return 0;
}

void diag(void){
	printf("-------start printing----------\n");
	sem_wait(&mut);	
	print(head);
	sem_post(&mut);
	printf("-------finish printing-------\n");
}

void *  diaprint(void* mis){
	pthread_mutex_lock(&mutex);
	count++;
	pthread_mutex_unlock(&mutex);
	struct itimerval it_val;
	if(signal(SIGALRM,(void(*)(int))diag)==SIG_ERR){
		fprintf(stderr,"unable to catch SIGALRM");
		exit(0);
	}
	it_val.it_value.tv_sec = 15000/1000;
	it_val.it_value.tv_usec = (15000*1000)%1000000;
	it_val.it_interval = it_val.it_value;
	if(setitimer(ITIMER_REAL,&it_val,NULL)==-1){
		fprintf(stderr,"error calling setitimer");
		exit(0);
	}
	while(1){
		pause();
	}
	exit(0);
}

void * handler(void* sock){

	pthread_mutex_lock(&mutex);
	count++;
	clientnum++;
	int t = clientnum-1;
	pthread_mutex_unlock(&mutex);
	fd[t] = *(int*)sock;
	char buffer[256];
	bzero(buffer,256);
	bzero(message,256);
	int n = -1;
	bool ifServer = false; 
	struct account* tmp = NULL;
	while((n = recv(fd[t],buffer,256,0))>0){
		bzero(message,256);		
//////////////////////////////////////////
		char * command;
		command = strtok(buffer," ");
		if(strcmp(command,"create")==0){
			if(ifServer == true){
				strcpy(message,"[S]Already in service");
				
			} else {
			command = strtok(0, "");
			sprintf(message,"[S]Trying to create a new account: %s",command);
			sem_wait(&mut);
			struct account* temp = head;
			//strcpy(temp,head -> name);
			head = createAccount(head,command);
			if(temp == NULL){
				strcat(message," | succeeded.");
			} else if(strcmp(head -> name,temp->name)== 0){
				strcat(message," | error in duplicate account name");
			} else {
				strcat(message," | succeeded");
			}
			
			sem_post(&mut);
			}
		} else if (strcmp(command,"serve")==0){
			if(head == NULL){
				strcpy(message,"[S]Error: cannot find account");
				
			} else if(ifServer == true){
				strcpy(message,"[S]Already in service");
			} else{
			sem_wait(&mut);
			tmp = head;
			command = strtok(0,"");
			while(tmp != NULL){
				if((strcmp(command,tmp -> name)== 0)&&(tmp->InSession == false)){
					strcpy(message,"[S]Got the account, open session");
					ifServer = true;
					tmp -> InSession = true;
					break;
				}
				tmp = tmp->next;
					
			}
			sem_post(&mut);
			if(ifServer == false)
				strcpy(message,"[S]Error: cannot find account or account opened");
			}
		} else if (strcmp(command,"quit")==0){
			if(ifServer == true){
				strcpy(message,"[S]cannot quit because in service");
			} else {
				strcpy(message, "quit");
			}
		} else if(strcmp(command,"deposit")==0){
			if(ifServer == false){
				strcpy(message, "[S]Error for not in session");
				
			} else{
			sem_wait(&mut);
			command=strtok(0,"");
			char * end;
			double amount = strtod(command,&end);
			if(changeBalance(tmp,amount) <0){
				sprintf(message, "[S]Error for under 0 balance");
			} else {
				sprintf(message, "[S]deposited your money");
			}
			sem_post(&mut);
			}
		} else if(strcmp(command,"withdraw")==0){
			if(ifServer == false){
				sprintf(message, "[S]Error for not in session");
				
			} else{
			sem_wait(&mut);
			command=strtok(0,"");
			char* end;
			double amount = -1 * strtod(command,&end);
			if(changeBalance(tmp,amount) <0){
				sprintf(message, "[S]Error for under 0 balance");
			} else {
				sprintf(message,"[S]withdrew your money");
			}
			sem_post(&mut);
			}
		} else if(strcmp(command,"query")==0){
			if(ifServer == false){
				sprintf(message, "[S]Error for not in session");
					
				
			} else{
			sem_wait(&mut);
				sprintf(message, "[S]current balance is %lf",tmp -> balance);
			sem_post(&mut);
			}
		} else if(strcmp(command,"end")==0){
			if(ifServer == false){
				strcpy(message, "[S]Error for not in session");
			
			} else {
			ifServer = false;
			tmp -> InSession = false;
			tmp = NULL;
			strcpy(message,"[S]Session ended");
			}
		} else {
			fprintf(stderr,"error in receiving command from client\n");
		}
////////////////////////////////////////////
		bzero(buffer,256);
		n = write(fd[t],message,strlen(message));
		bzero(message,256);

		if(n <0){
			fprintf(stderr,"[S]ERROR in write to socket\n");
			break;
		}
	}
	if(n == 0){
		printf("[S]Client disconnected\n");
		fflush(stdout);
	} else if(n < 0){
		fprintf(stderr,"[S]Error in receiving message\n");	
	}
	free(sock);
	pthread_exit(NULL);
	return;
}
