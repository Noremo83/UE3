// 161314 Andreas Kolan
#include<stdio.h> 
#include<stdlib.h>
#include<string.h> 
#include<fcntl.h> 
#include<sys/stat.h> 
#include<sys/types.h> 
#include<unistd.h> 
#include<errno.h>

#define MAX_LEN 1024
#define MIN_REQUIRED 1
#define MAX_OUTPUT 1024

char *wo_14(){	
	static char cwd[MAX_LEN];
	
	//Abfragen des Current Working Dir mit getcwd
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		//Ausgabe des Current Working Directory
    	return cwd;
	} 
	else {
		//Fehler sollte das CWD nicht abgefragt werden können
		return "ERROR get PATH";
   }
}

int main(int argc, char **argv) 
{ 
	int fd,fd2; 
	char * envpipe = "/var/spool/envpipe";
    char * mypipe = "/var/spool/14-cmdpipe"; 		
	char env[MAX_LEN];
	char output[MAX_OUTPUT];
	char *command = malloc(MAX_LEN);
	
	//Überprüfen ob ein Command mitgegeben wurde.
	if (argc <= MIN_REQUIRED){
		fprintf(stderr,"Zu wenig Argumente.\nBitte command angeben das abgesetzt werden soll");
		exit(1);
	}
	
	//Befüllen der Umgebungvariablen die mitgeliefert werden sollen
	snprintf(env,sizeof(env),"%i %i %i %s %s\n",getuid(),geteuid(),umask(S_IRWXG),getenv("HOME"),wo_14());
	printf("%s\n",env);
	
	//Befüllen eines Command Arrays zur übergabe der Befehle damit sie am Server gleich wie gewohnt durch leerzeichen getrennt verarbeitet werden können.
	int i;
	for(i = 1; i < argc; i++){
		strcat(command,argv[i]);
		strcat(command," ");
	}
	strcat(command,"\n");
 	printf("%s\n",command);    
	
	//Öffnen und schreiben der Environment Pipe
	if(mkfifo(envpipe,0666) == 0)				
		fd = open(envpipe, O_WRONLY);
	else if(errno == EEXIST)
		fd = open(envpipe, O_WRONLY);
	
  	write(fd, env, strlen(env)+1);
	close(fd); 		
	
	//Öffnen und schreiben der Command Pipe
	if(mkfifo(mypipe,0666) == 0)				
		fd2 = open(mypipe, O_WRONLY);
	else if(errno == EEXIST)
		fd2 = open(mypipe, O_WRONLY);
	
	write(fd2, command, strlen(command)+1);
	close(fd2); 	
	
			 
	//Öffnen der Pipe zum einlesen des Rückgabewertes
	fd2 = open(mypipe, O_RDONLY); 
	read(fd2, output, MAX_OUTPUT);
	close(fd2); 
	printf("%s",output);
		 
			 
	//Aufräumen	
	printf("done\n");
	free(command);
    return 0; 
} 

