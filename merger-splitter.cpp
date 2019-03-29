#include <iostream>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/wait.h>
#include <cmath>
#include "Record.h"
# include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

using namespace std;

void make_tree(int height,int start,int end,char* fil,char* sub,int root,int num=0);
void make_tree_s(int height,int recnum,int sum,int start,int end,char* fil,char* sub,int root,int num=0);
int h;
int main(int argc,char* argv[])
{
    int dsflag=0;
    char fname[50];
    char substring[200];
    for(int i=0;i<argc;i++)//argument passing
    {
        if(strcmp(argv[i],"-d")==0)
            strcpy(fname,argv[i+1]);
        else if(strcmp(argv[i],"-s")==0)
            dsflag=1;
        else if(strcmp(argv[i],"-h")==0)
            h=atoi(argv[i+1]);
        else if(strcmp(argv[i],"-p")==0)
            strcpy(substring,argv[i+1]);
    }
    Record rec;
    FILE *fp;
    long fsize;
    int recnum;
    fp = fopen (fname,"rb");
    if(fp==NULL)
    {
        cout<<"cannot open binary"<<endl;
        return -1;
    }
    fseek(fp,0,SEEK_END);
    fsize=ftell(fp);
    rewind(fp);
    recnum=(int)fsize/sizeof(rec);
    fclose(fp);//find file size;

    int searchernum=1;
    for(int i=0;i<h;i++)
        searchernum=searchernum*2;//find searchers number depending on the height that was given


    int parent=getppid();
    int cpid;
    if(h!=1)
        cpid=fork();//if height==1 then we do not need to create any more merger-splitters
    else cpid=0;//if it is !=1 then we need to create more, so we fork
    if (cpid==0)
    {
        if(dsflag==0) make_tree(h,1,(int)recnum,fname,substring,parent,1);//if skew form was not asked
        else
        {
            int sum=0;//calculate the sum (which is constant) for the skew form
            for(int i=1;i<=searchernum;i++)
            {
                sum=sum+i;
            }
            make_tree_s(h,(int)recnum,sum,1,searchernum,fname,substring,parent,1);//if skew form was asked
        }
        return 0;
    }
    else
    {
        char* s=new char[20];
        sprintf(s,"%d%d",getpid(),1);//named pipes are used for communication. names are pid and 0 or 1 for each child
        int f;
        f= mkfifo (s, 0666);//make fifo
        if((f==-1) &&( errno != EEXIST ))
        {
            cout<<"error creating named pipe"<<endl;
            return 1;
        }
        int fd,rd,w,fdp;
        Record r;
        char * sp=new char[20];
        fd=open(s, O_RDONLY);//open pipe for reading.
        if(fd==-1)
            cout<<"we have error"<<endl;
        sprintf(sp,"%d%d",getppid(),1);
        fdp=open(sp,O_WRONLY);//open pipe for writing to parent
        if(fdp==-1)
            cout<<" smerg error"<<endl;
        int i=-1;
        while(1)
        {
            i++;
            rd=read(fd,&r,sizeof(Record));//this blocks until child writes
            if(rd==0)
                break;
            w=write(fdp,&r,sizeof(Record));//this blocks until parent is ready to receive
            if(rd!=w)
                cout<<"different"<<endl;
        }
        close(fd);
        unlink(s);//close pipe to child
        delete[] s;
        delete[] sp;

    }
    return 1;
}

void make_tree(int height,int start,int end,char* fil,char* sub,int root,int num)
{
    if(height<1)
    {
        char** tmp;
        tmp=new char*[14];//if height < 1 then height is 0 so we need to run the searcher executable
        for(int i=0;i<13;i++)
        {
            tmp[i]=new char[50];
        }
        strcpy(tmp[0],"./searcher");//we load up the arguments
        strcpy(tmp[1],"-s");
        sprintf(tmp[2],"%d",start);
        strcpy(tmp[3],"-e");
        sprintf(tmp[4],"%d",end);
        strcpy(tmp[5],"-d");
        strcpy(tmp[6],fil);
        strcpy(tmp[7],"-p");
        strcpy(tmp[8],sub);
        strcpy(tmp[9],"-r");
        sprintf(tmp[10],"%d",root);
        strcpy(tmp[11],"-f");
        sprintf(tmp[12],"%d%d",getppid(),num);
        tmp[13]=(char*)NULL;
        execvp("./searcher",tmp);//and call the executable
        return;
    }
    time_t startc,endc;
    double timer;
    startc=clock();//start timer for merger-splitter
    int ch1=0,ch2=0;
    if((ch1=fork())==-1)//create child 1
    {
        cout<<"ch1 error"<<endl;
        return;
    }
    else if(ch1==0)
    {
        make_tree(height-1,(int)(end+start)/2+1,end,fil,sub,root,1);//recursive call for child 1
        return;
    }
    if((ch2=fork())==-1)//create child 2
    {
        cout<<"ch2 error"<<endl;
        return;
    }
    else if(ch2==0)
    {
        make_tree(height-1,start,(int)(end+start)/2,fil,sub,root,0);//recursive call for child 2
        return;
    }
    int f,fd1,fd2,l;
    char* c1=new char[20];
    char* c2=new char[20];
    int rd,fdp,w;
    Record r;
    char *sp=new char[20];
    sprintf(c1,"%d%d",getpid(),1);
    sprintf(c2,"%d%d",getpid(),0);
    f= mkfifo (c1, 0666);//create different pipe for each child so that they do not interfere for each other
    if((f==-1) &&( errno != EEXIST ))
    {
        cout<<"error creating named pipe"<<endl;
        return ;
    }
    l= mkfifo (c2, 0666);//for child 2;
    if((l==-1) &&( errno != EEXIST ))
    {
        cout<<"error creating named pipe"<<endl;
        return ;
    }
    fd1=open(c1, O_RDONLY);//open pipes for reading
    if(fd1==-1)
        cout<<"error this merg"<<endl;
    fd2=open(c2, O_RDONLY);//same here
    if(fd2==-1)
        cout<<"error this merg"<<endl;
    sprintf(sp,"%d%d",getppid(),num);
    fdp=open(sp,O_WRONLY);//open pipe for writing. (the parent is listening on the other end)
    if(fdp==-1)
        cout<<"error parent"<<endl;
    while(1)
    {
        rd=read(fd1,&r,sizeof(Record));//read from child one and write to parent
        if(rd==0)
            break;
        w=write(fdp,&r,sizeof(Record));
        if(rd!=w)
            cout<<"different"<<endl;
    }
    while(1)
    {
        rd=read(fd2,&r,sizeof(Record));//read from the other child and write to parent as well
        if(rd==0)
            break;
        w=write(fdp,&r,sizeof(Record));
        if(rd!=w)
            cout<<"different"<<endl;
    }
    close(fd1);//close pipes to children
    close(fd2);
    unlink(c1);
    unlink(c2);
    r.custid=0;
    strcpy(r.FirstName,"Merger-Splitter");
    endc=clock();//end timer and send stats to pipe
    timer=(double)(endc-startc)/CLOCKS_PER_SEC;
    r.amount=timer;
    write(fdp,&r,sizeof(Record));//write to pipe
    delete[] c1;
    delete[] c2;
    delete[] sp;
    return;
}
void make_tree_s(int height,int recnum,int sum,int start,int end,char* fil,char* sub,int root,int num)
{
    if(height<1)//if height < 1 then height is 0 so we need to run the searcher executable
    {
        double unow=0,s;
        for(int i=1;i<start;i++)//calculate the sum to pass as argument
        {
            s=(double)recnum*(double)i/(double)sum;
            unow=round(unow+s);
        }
        start=unow+1;
        end=round(unow+(double)recnum*((double)end)/(double)sum);
        char** tmp;
        tmp=new char*[14];
        for(int i=0;i<13;i++)//we load up the arguments
        {
            tmp[i]=new char[50];
        }
        strcpy(tmp[0],"./searcher");
        strcpy(tmp[1],"-s");
        sprintf(tmp[2],"%d",start);
        strcpy(tmp[3],"-e");
        sprintf(tmp[4],"%d",end);
        strcpy(tmp[5],"-d");
        strcpy(tmp[6],fil);
        strcpy(tmp[7],"-p");
        strcpy(tmp[8],sub);
        strcpy(tmp[9],"-r");
        sprintf(tmp[10],"%d",root);
        strcpy(tmp[11],"-f");
        sprintf(tmp[12],"%d%d",getppid(),num);
        tmp[13]=(char*)NULL;
        execvp("./searcher",tmp);//and call the searcher executable
        return;
    }
    time_t startc,endc;
    double timer;//start timer for merger-splitter
    startc=clock();
    int ch1=0,ch2=0;
    if((ch1=fork())==-1)//fork child 1
    {
        cout<<"ch1 error"<<endl;
        return;
    }
    else if(ch1==0)
    {
        make_tree_s(height-1,recnum,sum,(int)(end+start)/2+1,end,fil,sub,root,1);//recursive merger-splitter call for child 1
        return;
    }
    if((ch2=fork())==-1)//fork child 2
    {
        cout<<"ch2 error"<<endl;
        return;
    }
    else if(ch2==0)
    {
        make_tree_s(height-1,recnum,sum,start,(int)(end+start)/2,fil,sub,root,0);//recursive merger-splitter call for child 2
        return;
    }
    int f,fd1,fd2,l;
    char* c1=new char[20];
    char* c2=new char[20];
    int rd,fdp,w;
    Record r;
    char *sp=new char[20];
    sprintf(c1,"%d%d",getpid(),0);
    sprintf(c2,"%d%d",getpid(),1);
    f= mkfifo (c1, 0666);//create different pipe for each child so they do not interfere
    if((f==-1) &&( errno != EEXIST ))
    {
        cout<<"error creating named pipe"<<endl;
        return ;
    }
    l= mkfifo (c2, 0666);//pipe 2
    if((l==-1) &&( errno != EEXIST ))
    {
        cout<<"error creating named pipe"<<endl;
        return ;
    }
    fd1=open(c1, O_RDONLY);//open pipe to read
    if(fd1==-1)
        cout<<"error this merg"<<endl;
    fd2=open(c2, O_RDONLY);//open pipe to read for the other child
    if(fd2==-1)
        cout<<"error this merg"<<endl;
    sprintf(sp,"%d%d",getppid(),num);
    fdp=open(sp,O_WRONLY);//open pipe to write to parent
    if(fdp==-1)
        cout<<"error parent"<<endl;
    while(1)//read from child pipe and write to parent
    {
        rd=read(fd1,&r,sizeof(Record));
        if(rd==0)
            break;
        w=write(fdp,&r,sizeof(Record));
        if(rd!=w)
            cout<<"different"<<endl;
    }
    while(1)//same for other child all in blocking mode
    {
        rd=read(fd2,&r,sizeof(Record));
        if(rd==0)
            break;
        w=write(fdp,&r,sizeof(Record));
        if(rd!=w)
            cout<<"different"<<endl;
    }
    close(fd1);
    close(fd2);
    unlink(c1);//close pipes
    unlink(c2);
    r.custid=0;
    strcpy(r.FirstName,"Merger-Splitter");
    endc=clock();
    timer=(double)(endc-startc)/CLOCKS_PER_SEC;
    r.amount=timer;//calculate time it took and write it as a record
    write(fdp,&r,sizeof(Record));
    delete[] c1;
    delete[] c2;
    delete[] sp;
    return;
}
