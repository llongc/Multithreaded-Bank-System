#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<errno.h>
#include<limits.h>
#include<dirent.h>
#include<signal.h>
#include<pthread.h>
#include<stdbool.h>

struct account{
	char name[256];
	double balance;
	bool InSession; 
	struct account* next;
};

struct account* createAccount(struct account* head,char* name){
	if(check(head,name)==-1){
		//fprintf(stderr,"error in duplicate account name\n");
		return head;
		//return 0;
	}
	struct account* row = (struct account* )malloc(sizeof(struct account));
	strcpy(row->name,name);
	row -> balance = 0;
	row -> InSession = false;
	row -> next = head;
	head = row;
	return head;
	//return 1;
}

int changeBalance(struct account * row, double number){
	if(((row -> balance)+number)<0){
		return -1;
	} else {
		row -> balance = (row -> balance) + number;
		return 0;
	}
}

int check(struct account* head, char* name){
	if(head == NULL) return 0;
	struct account* tmp;
	tmp = head;
	while(tmp != NULL){
		if(strcmp(name,tmp->name)==0){
			return -1;			
		}
		tmp = tmp -> next;
	}
	return 0;
}

void freestruct(struct account* head){
	struct account* tmp;
	tmp = head;
	struct account* ptr = NULL;
	
	while(tmp != NULL){
		ptr = tmp;
		tmp = tmp->next;
		free(ptr);
	}
}

void print(struct account* head){
	struct account* tmp;
	tmp = head;
	while(tmp != NULL){
		char service[20];
		bzero(service,0);
		strcpy(service,"");
		if(tmp->InSession == true){
			strcpy(service,"\tIN SERVICE");
		}
		printf("%s\t%lf%s\n",tmp->name,tmp->balance,service);
		tmp = tmp->next;
	}
}
