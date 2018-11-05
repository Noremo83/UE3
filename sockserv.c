#include<sys/types.h>
#include<sys/socket.h>
#include<pthread.h>
#include<stdio.h>
#include <netinet/in.h>
#include<stdlib.h>
#include<unistd.h> 


void shell_for_client(int *fd){

	dup2(fd,0);
	dup2(fd,1);
	dup2(fd,2);
	close(fd);
	
	shell();
}

void * clhandler(void *p){
	int fd = (int)(p);
	
	char buf[30];
	
	FILE *fp = fdopen(fd,"r+");
	
	printf("Client mit FD %d connected\n",fd);
	
	fprintf(fp,"Hallo %d\n",fd);
	fgets(buf,20,fp);
	//write(fd,"HALLO DU\n",9);
	//hier eine schleife um die Nachrichten auf alle zu verteilen
	close(fd);
}

int main(){

	//Socket creation
	int sock, clsock, error,ret, claddrlen;	
	pthread_t thrd;
	struct sockaddr_in addr;
	struct sockaddr_in claddr;
	
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(2205);
	addr.sin_addr.s_addr = INADDR_ANY;	
	
	sock = socket(AF_INET,SOCK_STREAM,0);
	//Fehlerbehandlung ==-1
		
	ret = bind(sock,(struct sockaddr *)(&addr),sizeof(addr));
	//Fehlerbehandlung == -1
	
	ret = listen(sock,7);
	//Fehlerbehandlung  == -1
	printf("Server läuft\n");
	
	for(;;){
		claddrlen = sizeof(claddr);
	
		clsock = accept(sock,(struct sockaddr *)(&claddr),&claddrlen);
		//Fehlerbehandlung
		
		//Client Adresse speichern THREADs
		pthread_create(&thrd,0,clhandler,(void *)clsock);
		
		//Für 2. Übung
		switch(fork()){
			case 0:
				shell_for_client(clsock);
				exit(0);
			default:
				
		
		}
	}
		
	return 0;
	
}