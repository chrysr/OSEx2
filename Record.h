#ifndef RECORD_H
#define RECORD_H
#define SIZEofBUFF 20
#define SSizeofBUFF 6
#include <cstring>
#include <iostream>
#include <cstdio>

using namespace std;

class Record{//the way the record was asked to be defined
public:
	long  	custid;
	char 	FirstName[SIZEofBUFF];
	char 	LastName[SIZEofBUFF];
	char	Street[SIZEofBUFF];
	int 	HouseID;
	char	City[SIZEofBUFF];
	char	postcode[SSizeofBUFF];
	float  	amount;
	int compare(char* inpt);
};

#endif
