#ifndef _database_h
#define _database_h

struct account{
	char name[256];
	double balance;
	bool InSession;
	struct account* next;
};

struct account* createAccount(struct account*, char*);

int changeBalance(struct account*, double);

int check(struct account*, char*);

void freestruct(struct account*);

void print(struct account*);

#endif
