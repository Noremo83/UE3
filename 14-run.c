// 161314 Andreas Kolan
#include<stdio.h> 
#include<stdlib.h>
#include<string.h> 
#include<fcntl.h> 
#include<sys/stat.h> 
#include<sys/types.h> 
#include<unistd.h> 

#define MAX_LEN 255
#define MIN_REQUIRED 1

int main(int argc, char **argv) 
{ 
	int fd,fd2; 
	char * envpipe = "/var/spool/envpipe";
    char * mypipe = "/var/spool/14-cmdpipe"; 		
	char env[MAX_LEN];
	char *command = malloc(MAX_LEN);
	
	//Überprüfen ob ein Command mitgegeben wurde.
	if (argc <= MIN_REQUIRED){
		fprintf(stderr,"Zu wenig Argumente.\nBitte command angeben das abgesetzt werden soll");
		exit(1);
	}
	
	//Befüllen der Umgebung die Mitgeliefert wird
	snprintf(env,sizeof(env),"%i %i %i %s\n",getuid(),geteuid(),umask(S_IRWXG),getenv("HOME"));
	printf("%s\n",env);
	
	//Befüllen eines Command Arrays zur übergabe der Befehle damit sie am Server gleich wie gewohnt verarbeitet werden können.
	int i;
	for(i = 1; i < argc; i++){
		strcat(command,argv[i]);
		//Abfangen letztes leerzeichen
		strcat(command," ");
	}
	strcat(command,"\n");
 	printf("%s\n",command);
    
	// Open Environmentpipe for write only 
	// Write the environment to pipe and close
    fd = open(envpipe, O_WRONLY); 
  	write(fd, env, strlen(env)+1);
	close(fd); 		
	
    // Open commandpipe for write only 
	// Write the command to pipe and close
	fd2 = open(mypipe, O_WRONLY); 
	write(fd2, command, strlen(command)+1);
	close(fd2); 	
	
	//Aufräumen	
	printf("done\n");
	free(command);
    return 0; 
} 