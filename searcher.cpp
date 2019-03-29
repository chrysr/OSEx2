#include <iostream>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "Record.h"
# include <fcntl.h>
#include <cstdio>
#include <ctime>
#include <errno.h>



time_t startc,endc;
double timer;
int ff;
using namespace std;
int searcherf(int start,int end,char* fil,char* sub,int root);
int main(int argc,char* argv[])
{
    startc=clock();
    int s,e;
    char fname[50];
    char substring[200];
    int root;
    for(int i=0;i<argc;i++)//handle input paremeters
    {
        if(strcmp(argv[i],"-d")==0)
            strcpy(fname,argv[i+1]);
        else if(strcmp(argv[i],"-s")==0)
            s=atoi(argv[i+1]);
        else if(strcmp(argv[i],"-e")==0)
            e=atoi(argv[i+1]);
        else if(strcmp(argv[i],"-p")==0)
            strcpy(substring,argv[i+1]);
        else if(strcmp(argv[i],"-r")==0)
            root=atoi(argv[i+1]);
        else if(strcmp(argv[i],"-f")==0)
            ff=atoi(argv[i+1]);
    }
    searcherf(s,e,fname,substring,root);//and call searcherf()

}
int searcherf(int start,int end,char* fil,char* sub,int root)
{
    FILE* fp;
    fp=fopen(fil,"rb");
    Record tmp;
    if(fp==NULL)
    {
        cout<<"cannot open binary"<<endl;
        return -1;
    }
    fseek(fp,start*sizeof(tmp),SEEK_SET);//open binary and start reading from (start)variable that was given by the parent
    int fd;//until the (end) variable also given by the parent
    char* s=new char[20];
    sprintf(s,"%d",ff);
    fd=open(s,O_WRONLY);//open pipe to write to parent
    if(fd==-1)
    {
        while(1)//this gives the system time to open the files. when height is 4 or 5, a big number of
        {//searchers is created and all of them try to open a file which causes the system to return
            sleep(0.5);//-1. So by doing this, any process that didn't succeed can try again, after waiting half a second
            fd=open(s,O_WRONLY);
            if(fd!=-1)
                break;
        }
        if(fd==-1)
            cout<<"error search"<<endl;

    }
    int sum=0;
    int wrote=0;
    for(int i=start;i<=end;i++)
    {
        fread(&tmp,sizeof(tmp),1,fp);//read from binary file
        if(tmp.compare(sub))
        {
            wrote+=write(fd,&tmp,sizeof(Record));//write to parent pipe
            sum++;
        }
    }
    fclose(fp);
    kill(root,SIGUSR2);//send SIGURS2 signal to parent
    tmp.custid=0;
    strcpy(tmp.FirstName,"Searcher");
    endc=clock();
    timer=(double)(endc-startc)/CLOCKS_PER_SEC;//calculate timer
    tmp.amount=timer;
    write(fd,&tmp,sizeof(Record));//write it to parent
    close(fd);//close pipe
    delete[] s;
    return 1;

}
