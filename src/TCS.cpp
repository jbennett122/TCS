#define SA struct sockaddr
#define BUFFER_SIZE 255
#define MAXCONNECTIONS 100

#include <stdio.h>
#include <regex.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <math.h>
#include <netdb.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <sys/wait.h>
#include <cstdint>

void SIGhandler(int sig);
void printList(int L[]);
void *chatThread(void *sockfd);
void *infoThread(void *sockfd);
void writeToPorts(char buffer[], int nread);

using namespace std;

int chatPort = 10907;
int infoPort=10908;

struct clientInfo{

	int fd;
	char ip[];
	int portNumber;

};

clientInfo L[MAXCONNECTIONS];		//array for connection info for information call
int connections =0;

int main(void)
{	int i = 0;
	int connfd;
	int len, listenfd, infofd;
	struct sockaddr_in servaddr, cliaddr,info;
	int yes=1;

	pthread_t       tid,tid2;
	pthread_attr_t  attr;

	//flush out the memory
	memset(&servaddr, 0, sizeof (servaddr));

	signal(SIGINT, SIGhandler);

	//chat attributes
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
	servaddr.sin_port = htons (chatPort);

	//info attributes
	info.sin_family = AF_INET;
	info.sin_addr.s_addr = htonl (INADDR_ANY);
	info.sin_port = htons (infoPort);

	 //create sockets
	 if ( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
	        printf ("error\n");
	        exit(-1);
	    }
	 if ( (infofd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
	 	        printf ("error\n");
	 	        exit(-1);
	 	    }
	 setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR, &yes,sizeof(int));
	 //clear sockets if inuse from before
	 if (setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR, &yes,sizeof(int)) != 0) {
		  cerr<<"1"<<endl;
		  perror("setsockopt");
			exit(1);
			}
	 setsockopt(infofd,SOL_SOCKET,SO_REUSEADDR, &yes,sizeof(int));
	  if (setsockopt(infofd,SOL_SOCKET,SO_REUSEADDR, &yes,sizeof(int)) != 0) {
			cerr<<"2"<<endl;
		  perror("setsockopt");
			exit(1);
			}

	//bind sockets to ports
	if (bind (listenfd, (SA *)&servaddr, sizeof(servaddr)) == -1)
	{	perror ("error binding\n");
		exit (-1);
	}
	if (bind (infofd, (SA *)&info, sizeof(info)) == -1)
		{	perror ("error binding\n");
			exit (-1);
		}


	int chatp=	listen(listenfd, 0);

	int infop =listen(infofd,0);



	//initalize thread with preset attributes
	pthread_attr_init(&attr);

	cout<<"listening for chat clients on port:"<<chatPort<<endl;

	cout<<"listening for info requests on port:"<<infoPort<<endl;


	for(;;){

	len = sizeof (cliaddr);
	//cerr<<"loop test"<<endl;
	connfd = accept (listenfd, (SA *) & cliaddr, ((socklen_t*)&len));

	cout<<"file descriptor created: "<<connfd<<endl;
	cout<<"IP PORT: "<<cliaddr.sin_port<<endl;
	L[i].portNumber=cliaddr.sin_port;
	L[i].fd=connfd;
	i++;
	connections++;
	cout<<i<<" different connections detected"<<endl;
	//create thread using new connfd
	pthread_create(&tid,&attr,&chatThread,(void*)connfd);

}

	listen(infofd,0);

	//initalize thread with preset attributes
	pthread_attr_init(&attr);

	for(;;){

	len = sizeof (cliaddr);
	// cerr<<"loop test"<<endl;
	connfd = accept (infofd, (SA *) & cliaddr, ((socklen_t*)&len));

	pthread_create(&tid2,&attr,&infoThread,(void*)connfd);
	//create thread using new connfd


}




pthread_join(tid, NULL);
pthread_join(tid2, NULL);
close(connfd);


}
void *chatThread(void *sockfd){
	 int             nread;
	    char            buffer[512];
	    for (;;) {

	        nread = read((intptr_t) sockfd, buffer, 512);

	        if (nread == 0) pthread_exit(0);

	        writeToPorts(buffer, nread);
	        write(1, buffer, nread);
	    }

}


//pass the ClientInfo array in to be written to all users connected
void *infoThread(void *sockfd){
	 cerr<<"test info thread\n"<<endl;
}

void writeToPorts(char buffer[], int nread){
int j;
//cout<<L[0].fd<<endl;;

write(1,buffer,nread);
if(buffer){
for(j=0;j<connections;j++){
	cout<<"connections"<<connections<<endl;
	cout<<"File descriptor for each:"<< L[j].fd<<endl;
	cout<<"Port Number:"<<L[j].portNumber<<endl;
	cout<<"IP:"<<L[j].ip<<endl;
	write(L[j].fd, buffer, nread);

	}

}
}
void printList(int L[]){


	;
}

void SIGhandler(int sig)
{	char msg[512]="\nbyebye";

for(int j=0;j<connections;j++){
	write(L[j].fd, msg, sizeof(msg));
	}
kill(getpid(),9);
}


