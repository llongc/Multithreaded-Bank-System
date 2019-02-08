#include<stdio.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<sys/socket.h>
#include<sys/time.h>
#include<fcntl.h>
#include<signal.h>
#include<semaphore.h>
#include<stdbool.h>
#include<pthread.h>

//pthread_mutex_t mutex = PHTREAD_MUTEX_INITIALIZER;
pthread_t thread1;
pthread_t thread2;
int sock = 0;
char line[256];
char buffer[256];
void * sendhandle (void* sok){
	int sk = *(int*)sok;
	bzero(line,256);
	while(strcmp(line,"quit")!=0){
		if(sock<0){
			break;
		}
		printf("[C]Please enter: ");
		scanf("%[^\n]%*c", line);
		fflush(stdout);
		send(sk,line,strlen(line),0);
		bzero(line,256);
	}
	pthread_exit(NULL);
	return;
}

void * respondhandle (void* sok){
	int sk = *(int*)sok;
	bzero(buffer,256);
	int valread;
	while((read(sk,buffer,255))>=0){
		printf("%s\n", buffer);
		if(strcmp(buffer,"quit")==0){
			printf("[C]disconnects from the server\n");
			fflush(stdout);
			sock = -1;
			exit(0);	
		}
		bzero(buffer,256);

	}
	return;
}
double check_num(char* target){
	int i, jud = 1, count = 0;
	if(target[0] =='.' || target[strlen(target)-1]=='.') jud = 0;
	for(i = 0; i < strlen(target); i++){
		if(target[i] =='.') count ++;
		int a = (int)target[i];
		if(a < 48 || a > 57){
			if(a != 46) jud = 0;
		}

	}
	if (count > 1) jud = 0;
	return jud;

}

double convert(char* buffer){
	char * end;
	double out = strtod(buffer,&end);
	return out;

}


int subcheck(char* str1, char* str2){
	char str[1000];
	strcpy(str, str1);
	str[strlen(str2)]='\0';
	if(strcmp(str,str2)==0){
		return 1;
	}
	return 0;
}

void get_buffer(char* target, char* operator){
	int loop = strlen(target)-strlen(operator);
	char temp[1000];
	strcpy(temp,"");
	int i = 0;
	for(i = 0; i<loop-1;i++){
		temp[i] = target[i+strlen(operator)+1];
	}
	temp[i]='\0';
	strcpy(target,temp);
}
int checkFormat(char* buffer){
	if(strcmp(buffer,"quit")==0){
		return 0;
	} else if(strcmp(buffer,"end")==0){
		return 0;
	} else if(strcmp(buffer,"query")==0){
		return 0;
	} else if(strlen(buffer)<7){
		return -1;
	} else if(subcheck(buffer,"serve")){
		get_buffer(buffer,"serve");
		return 0;
	} else if(strlen(buffer)<8){
		return -1;
	} else if(subcheck(buffer,"create")){
		get_buffer(buffer,"create");
		return 0;
	} else if(strlen(buffer)<9){
		return -1;
	} else if(subcheck(buffer,"deposit")){
		get_buffer(buffer,"deposit");

		if(check_num(buffer)){
		} else {
			fprintf(stderr,"error: not a number\n");

		}
		return 0;
	} else if(strlen(buffer)<10){
		return -1;
	} else if(subcheck(buffer,"withdraw")){
		get_buffer(buffer,"withdraw");

		if(check_num(buffer)){
		} else {
			printf("error: not a number\n");

		}
		return 0;
	} else {
		return -1;
	}

}

int main(int argc, char* argv[]){
	struct sockaddr_in address;
	int sock = 0;
	int port = 55555;	

	struct sockaddr_in serv_addr;
	char line[256];
	bzero(line,256);
	char buffer[256];
	bzero(buffer,256);
	if(argc != 3){
		fprintf(stderr,"usage %s hostname port\n",argv[0]);
		exit(0);
	}
	const char* hostname = argv[1];
	port = atoi(argv[2]);
	if((sock = socket(AF_INET,SOCK_STREAM,0)) <0){
		fprintf(stderr,"socket creation error\n");
		exit(0);
	}
	bzero((char*)&serv_addr,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);	
	struct hostent *he = gethostbyname(hostname);
	serv_addr.sin_addr = *((struct in_addr *) he -> h_addr);
	//serv_addr.sin_addr.s_addr=gethostbyname("null.cs.rutgers.edu");
	/**if(inet_pton(AF_INET,gethostbyname("null.cs.rutgers.edu"),&serv_addr.sin_addr)<=0){
	}
	**/	
	while(connect(sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr))){
		fprintf(stderr,"Connection failed,try again\n");
		//exit(0);
		sleep(3);
	}
	int* sk = (int*)malloc(1);
	*sk = sock;	
	if(pthread_create(&thread2,NULL,respondhandle,(void*)sk)<0){
		fprintf(stderr,"[C]ERROR on create respond handler\n");
		exit(0);
	}
	printf("[C]Completion of connection to server\n");
	fflush(stdout);
	int i = 0;
	while(1){		
		if(i != 0){
			sleep(2);
		}
		i++;		
		printf("[C]Please enter: ");
		scanf("%[^\n]%*c",line);
		char temp[1000];
		strcpy(temp,line);
		if(checkFormat(temp)<0){
			printf("[C]Error in command format\n");
			fflush(stdout);
			bzero(line,256);
			continue;
		}
		send(sock,line,strlen(line),0);
	}
	pthread_cancel(thread2);
	close(sock);
	pthread_join(thread2,NULL);
	//close(sock);
	printf("[C]END\n");
	fflush(stdout);
	return 0;	
}
