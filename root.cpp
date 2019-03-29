#include <iostream>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "Record.h"
#include <cstdio>
# include <poll.h>
# include <signal.h>
# include <ctype.h>
# include <fcntl.h>
#include <sys/stat.h>
#include <ctime>
#include <errno.h>
using namespace std;

volatile sig_atomic_t variable=0;
void handler(int sig)
{
    signal(SIGUSR2,handler);//signal handler for signals sent by searchers
    variable++;
}
int main(int argc,char* argv[])
{
    time_t start,end;
    double timer;
    start=clock();//start timer for root
    signal(SIGUSR2,handler);
    int h,dsflag=0;
    char fname[50];
    char substring[50];
    int displayflag=1;
    char outfile[50];
    strcpy(outfile,"output.txt");
    int allok=0;
    if(argc >=7&&argc<=11)//arguments passed to program
    {
        for(int i=0;i<argc;i++)
        {
            if(strcmp(argv[i],"-d")==0)
                strcpy(fname,argv[i+1]);
            else if(strcmp(argv[i],"-s")==0)
                dsflag=1;
            else if(strcmp(argv[i],"-h")==0)
                h=atoi(argv[i+1]);
            else if(strcmp(argv[i],"-p")==0)
                strcpy(substring,argv[i+1]);
            else if(strcmp(argv[i],"-noshow")==0)
                displayflag=0;
            else if(strcmp(argv[i],"-f")==0)
                strcpy(outfile,argv[i+1]);
        }
        if(h>=1&&h<=5)
            allok=1;
    }
    if(allok)//if all required paremeters have been given then continue
    {
        int searchernum=1;
        for(int i=0;i<h;i++)
            searchernum=searchernum*2;
        cout<<"*************Program Started*************"<<endl;
        cout<<"-Will Create "<<searchernum<<" searchers for height "<<h<<endl;
        if(dsflag)
            cout<<"-Search mode is skew"<<endl;
        else cout<<"-Search mode is normal"<<endl;
        cout<<"-Looking for pattern |"<<substring<<"|"<<endl;
        cout<<"-Results will be written to file: <"<<outfile<<">"<<endl<<endl<<endl;
    }
    else//if something is not right with parameters prin help message
    {
        cout<<"This program created a Binary Tree of Processes"<<endl;
        cout<<"and searches for a specific pattern in a Binary File"<<endl<<endl;
        cout<<"Usage: "<<argv[0]<<" [OPTIONS]..."<<endl<<endl;
        cout<<"Mandatory Arguments: "<<endl;
        cout<<"\t-d <filename>      Input Binary File"<<endl;
        cout<<"\t-h <height>        Height of Binary Tree to be created (1-5)"<<endl;
        cout<<"\t-p <pattern>       Pattern to look for in the Binary Tree"<<endl;
        cout<<endl<<"Optional Arguments"<<endl;
        cout<<"\t-s                 Look for pattern in skew form"<<endl;
        cout<<"\t-f <filename>      Save output to specific file. Default is: <output.txt>"<<endl;
        cout<<"\t-noshow            Do not show output to terminal. Not enabled by default"<<endl;
        return -1;
    }
    int cpid=fork();
    if(cpid==-1)
        cout<<"fork error"<<endl;
    if (cpid==0)
    {
        char **tmp;
        tmp=new char*[argc+1];
        tmp[0]=new char[50];//create a child from the root
        strcpy(tmp[0],"./merger-splitter");
        int i;
        for(i=1;i<argc;i++)
        {
                tmp[i]=new char[50];
                strcpy(tmp[i],argv[i]);
        }
        tmp[argc]=(char*)NULL;//pass the correct arguments
        execvp("./merger-splitter",tmp);//and call execvp() to overlay the current program;
        return 1;
    }
    char* s=new char[20];
    int rd,fd,f;
    Record r;
    FILE* fpp;
    sprintf(s,"%d%d",getpid(),1);
    f= mkfifo (s, 0666);//creating a named pipe so that the child that was forked can write any results
    if((f==-1) &&( errno != EEXIST ))
    {
        cout<<"error creating named pipe"<<endl;
        return 0;
    }

    fd=open(s, O_RDONLY);//open pipe for reading
    if(fd==-1)
        cout<<"main error"<<endl;
    fpp=fopen(outfile,"w");//open and output file to write the results
    if(fpp==NULL)
        cout<<"null"<<endl;
    double mins=999999,maxs=0,avgs=0;
    double minm=999999,maxm=0,avgm=0;
    int cnt=0,cntm=0,cnts=0;
    double sum=0;
    while(1)
    {
        rd=read(fd,&r,sizeof(Record));//this will block until the root's child starts writing something to the fifo
        if(rd==0)
            break;
        if(r.custid==0)
        {
            sum+=r.amount;
            if(strcmp(r.FirstName,"Searcher")==0)//trick used for searchers and merger-splitters to send their times
            {
                if(r.amount<mins)
                    mins=r.amount;
                if(r.amount>maxs)
                    maxs=r.amount;
                avgs+=r.amount;
                cnts++;
            }
            else if(strcmp(r.FirstName,"Merger-Splitter")==0)
            {
                if(r.amount<minm)
                    minm=r.amount;
                if(r.amount>maxm)
                    maxm=r.amount;
                avgm+=r.amount;
                cntm++;
            }
            continue;
        }
        cnt++;//write matches to output file
        fprintf(fpp,"%ld %s %s  %s %d %s %s %-9.2f\n", \
        r.custid, r.LastName, r.FirstName, \
        r.Street, r.HouseID, r.City, r.postcode, \
        r.amount);
    }
    avgs=avgs/cnts;
    avgm=avgm/cntm;
    fclose(fpp);//close output file
    cpid=fork();
    if(cpid==-1)
    {
        cout<<"error sorting"<<endl;
        return -1;
    }
    else if(cpid==0)
    {
        execlp("sort","sort","--output",outfile,outfile,NULL);
    }
    close(fd);
    unlink(s);//close and unlink fifo
    delete[] s;
    wait(NULL);//wait for first merger splitter
    wait(NULL);//wait for sort
    if(displayflag)
    {
        fpp=fopen(outfile,"r");
        char buf[200];
        while(fgets(buf,sizeof(buf),fpp))
        {
            cout<<buf;
        }
        fclose(fpp);
    }
    end=clock();//end timer
    timer=(double)(end-start)/CLOCKS_PER_SEC;
    sum+=timer;//print Statistics for the program
    printf("Root\n\tTime: %.6f seconds\n\tReceived %d SIGUSR2 signals\n\n",timer,variable);
    printf("Merger-Splitters (%d)\n\tMIN: %.6f seconds\n\tMAX: %.6f seconds\n\tAVG: %.6f seconds\n\n",cntm,minm,maxm,avgm);
    printf("Searchers (%d)\n\tMIN: %.6f seconds\n\tMAX: %.6f seconds\n\tAVG: %.6f seconds\n\n",cnts,mins,maxs,avgs);


    cout<<"-Found "<<cnt<<" matches"<<endl;
    cout<<"-Turnaround Time: "<<sum<<" seconds"<<endl;
    cout<<"**************Program Ended**************"<<endl;

    return 1;

}
