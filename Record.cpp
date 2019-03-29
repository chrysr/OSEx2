#include "Record.h"

int Record::compare(char* inpt)
{//for each one of the fields, if the given input is a substring of any, return true, otherwise false
    char custi[30];
    char Housei[20];
    char amoun[30];
    sprintf(custi,"%ld",custid);
    sprintf(Housei,"%d",HouseID);
    sprintf(amoun,"%f",amount);
    if(strstr(custi,inpt)!=NULL)
        return 1;
    else if(strstr(FirstName,inpt)!=NULL)
        return 1;
    else if(strstr(LastName,inpt)!=NULL)
        return 1;
    else if(strstr(Street,inpt)!=NULL)
        return 1;
    else if(strstr(Housei,inpt)!=NULL)
        return 1;
    else if(strstr(City,inpt)!=NULL)
        return 1;
    else if(strstr(postcode,inpt)!=NULL)
        return 1;
    else if(strstr(amoun,inpt)!=NULL)
        return 1;
    else return 0;
}
