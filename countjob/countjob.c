//IS16134  Andreas Kolan

#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h> 
#include<sys/wait.h> 
#include<unistd.h> 
#include<signal.h>
#include<time.h>
#include<pthread.h>
#include"rawio.h"

#define MAX_INPUT 255
#define MAX_PATH 255
#define MAX_WORDS 25

void shell();
char **split(char *str,char *delim);
void end_14();
char *wo_14();
void cd();
void info_14();
void getpath();
void setpath_14(const char *path);
void addtopath_14(const char *path);
char *checkBackground(char *str);
void printhelp();

void sigchld_handler(int signo);
void sigint_handler(int signo);

void print_proc();

char **cmdv;
int runBackground = 0;
int proc_count;
int writetoscreen = 1;

void sigint_handler(int signo) {
	printf("\n");
    exit(1);
}

void sigchld_handler(int signo) {
	pid_t pid;
	pid = wait(NULL);
	printf("\nPid %d exited.\n", pid);
	//wait(NULL);
	--proc_count;
}

//Startet die shell Funktion
int main(){
	//Proccess Counter initialisieren und Thread starten
	proc_count = 0;
	pthread_t threadID;
	pthread_create(&threadID,NULL,(void *)print_proc,NULL);	
	
	shell();	
	return 0;
}

void print_proc(){
	for(;;){
		//Kontrolle ob vorder oder Hintergrundprozess und je nachdem Ausgabe aktivieren oder nicht
		if(writetoscreen == 0){
			//Zeit holen
			time_t rawtime = time(0);		
			struct tm *local = localtime(&rawtime);		
			char retval[100];
			//speichern des Strings für die Ausgabe in der Variable retval und anschließend ausgeben 
			snprintf(retval,sizeof(retval),"\n161314--%i--%02i:%02i:%02i\n",proc_count, local->tm_hour, local->tm_min, local->tm_sec);
			writestr_raw(retval,20,0);	
			//printf("\n161314--%i--%02i:%02i:%02i\n",proc_count, local->tm_hour, local->tm_min, local->tm_sec);
		}
		sleep(1);
	}
}

void printhelp(){
	//Liefert eine Liste der verfügbaren Kommandos zurück
	printf("\nBuild in Funktionen:\n\n    14-ende - Beendet die Shell\n    14-wo - liefert das Working Directory\n    14-info - liefert Systeminformationen\n");
	printf("    getpath - liefert die PATH Variable\n    14-setpath Pfad - überschreibt die PATH Variable mit dem gelieferten Pfad\n    14-addtopath Pfad - fügt den angegeben Pfad der PATH Variable hinzu\n");
}

//Beendet das programm
void end_14(){
	printf("Hope to see you again!\n\n");
	exit(1);
}

//PWD Funktion
char *wo_14(){	
	static char cwd[MAX_PATH];
	
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

void cd(){
	char cwd[MAX_PATH];	
	//Falls das 2 Element der angegebenen Parameter leer sein Fehlermeldung gültigen Pfad angeben
	if (cmdv[1] != NULL){		
		if (getcwd(cwd, sizeof(cwd)) != NULL) {
			chdir(cmdv[1]);
			wo_14();
		} 
		else {
			//Fehler sollte das neue Working Directory nicht gewechselt werden können
			perror("Couldn´t find folder!\n");
	   }
	}
	else
		printf("Enter a valid path to switch!\n");
}

void info_14(){
	//Liefert Systeminformationen
	printf("\nSysteminfo:\n");
	printf("    UID: %d\n",getuid());
	printf("    EUID: %d\n",geteuid());
	printf("    PID: %d\n",getpid());
	printf("    CWD: %s\n",wo_14());
	printf("    PATH: %s\n\n",getenv("PATH"));
}

void setpath_14(const char *path){
	//Einfach überschreiben der bestehenden PATH Variable mit setenv und ausgebe
	setenv("PATH",path,1);  
	printf("PATH=%s\n",getenv("PATH"));
}

void getpath(){
	//Liefert die aktuelle PATH Variable zurück
	printf("PATH: %s\n",getenv("PATH"));
}

void addtopath_14(const char *addpath){
	//PATH Variable laden und einen : anhängen
	char *path = getenv("PATH");
	char *newpath = strcat(path,":");
	//Speichern und ausgeben der neuen PATH Variable
	setenv("PATH",strcat(newpath,addpath),1);   
	printf("PATH %s\n",getenv("PATH"));	
}
	
char **split(char *str,char *delim){
	int i = 0;	
	//Temporäres Array zum speichern der Commandline Argumente
	char **tmparr= malloc(16 * sizeof(char *));
	//FM im falle das malloc fehlschlägt
	if (tmparr == NULL) {
        perror("Malloc failed!\n");
        exit(1);
    }
	
	//initialisieren strtok und speichern in einer temp variablen
	char *tmp = strtok(str,delim);	
	while(tmp != NULL)	{
		//anschließend nach den definierten delimitern trennen und ins tmparray speichern	
		tmparr[i++]= tmp;
		tmp = strtok(NULL, delim);
	}
	//Sicherheitshalber am Ende noch eine NULL hinzufügen für kontroll zwecke die eventuell später benötigt werden
	tmparr[i] = NULL;
	return tmparr;
}

char *checkBackground(char *str){
	//Kontrolliert ob 1 Character ein & ist und somit der Prozess im Hintergrund laufen soll
	//Anschließend wird beim Argument noch das & entfernt und der Bool Wert von runBackground auf 1 gesetzt
	if(str[0] == '&'){
		runBackground = 1;	
		char *tmp = str + 1;
		str = tmp;
	}
	//Soll nicht im Hintergrund laufen = 0
	else
		runBackground = 0;
	return str;
}

void shell(){
	char eingabe[MAX_INPUT];
	int stat_loc;
	
	printf("\nStart my Shell\n\n");	
	
	//Signale ignorieren
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGCHLD, sigchld_handler);
	
	
	for(;;){		
		runBackground = 0;
		printf("14-%s:>",wo_14());		
		
		//Einlesen des Userinputs
		//fgets(eingabe,MAX_INPUT,stdin);
		
		gets_raw(eingabe,MAX_INPUT,0,get_lines());
		
		//Abfangen falls ein leerer Input erfolgt
		if((eingabe[0] == '\n') || (eingabe[0] == '\t')){
			continue;
		}
		//Argumente zerlegen
		cmdv = split(eingabe," \t\n");
		
		//Kontrolle ob Hintergrund prozess und entfernen des &
		cmdv[0] = checkBackground(cmdv[0]);		
		
		//Abfrage ob eine Build in Option benutzt werden soll und diese anschließend ausführen
		if(strcmp(cmdv[0],"14-ende") == 0 || strcmp(cmdv[0],"end") == 0){
			free(cmdv);
			runBackground = 0;
			end_14();
		}
		else if(strcmp(cmdv[0],"14-wo") == 0){
			printf("CWD: %s\n",wo_14());
			free(cmdv);
			runBackground = 0;
			continue;
		}		
		else if(strcmp(cmdv[0],"cd") == 0){
			cd();
			free(cmdv);
			runBackground = 0;
			continue;
		}		
		else if(strcmp(cmdv[0],"14-info") == 0 || strcmp(cmdv[0],"info") == 0){
			info_14();
			free(cmdv);
			runBackground = 0;
			continue;
		}
		else if(strcmp(cmdv[0],"14-addtopath") == 0 && cmdv[1] != NULL){
			addtopath_14(cmdv[1]);
			free(cmdv);
			runBackground = 0;
			continue;
		}
		else if(strcmp(cmdv[0],"14-setpath") == 0 && cmdv[1] != NULL){
			setpath_14(cmdv[1]);
			free(cmdv);
			runBackground = 0;
			continue;
		}
		else if(strcmp(cmdv[0],"getpath") == 0){
			getpath();
			free(cmdv);
			runBackground = 0;
			continue;
		}
		else if (strcmp(cmdv[0],"help") == 0){
			printhelp();
			free(cmdv);
			runBackground = 0;
			continue;
		}
		//Start Childprocess
		pid_t child;
		writetoscreen = 0;
		switch(child=fork()){
			case -1: perror("fork");
				break;
			case 0: //Childprocess
				//Kontrolle ob Hintergund (1 = Hintergrund) oder nicht
				//Signale für den ChildProcess richtigstellen
				if(runBackground == 1){	
					++proc_count;
					//Wenn Hintergrund dann Ignore Signale					
					signal(SIGINT, SIG_IGN);
					signal(SIGQUIT, SIG_IGN);
					
					//SIGCHLD handler aktivieren
					signal(SIGCHLD,sigchld_handler);
					
					//Ausführen des übergebenen Programms
					if(execvp(cmdv[0],cmdv) < 0){
						perror(cmdv[0]);
						exit(1);
					}					
				}
				else{
					//Wenn kein Hintergrund dann Standardverhalten Signale					
					signal(SIGINT, SIG_DFL);
					signal(SIGQUIT, SIG_DFL);
					
					//Signak CHLD auf IGN stellen sonst zählt er falsch
					signal(SIGCHLD,SIG_IGN);
					
					//Ausführen des übergebenen Programms
					if(execvp(cmdv[0],cmdv) < 0){
						perror(cmdv[0]);
						exit(1);
					}
				}
				
			default: //Bin ich selber und ggfs warten auf beendigung des childprocess
				if(runBackground == 1){	
					//++proc_count;
					
					//Wenn Hintergrund dann Ignore Signale und lauf weiter zur neuerlichen befehleingabe					
					signal(SIGINT, SIG_IGN);
					signal(SIGQUIT, sigint_handler);
					
					//SIGCHLD handler aktivieren
					signal(SIGCHLD,sigchld_handler);					
				}
				else{					
					//Wenn kein Hintergrund dann Standardverhalten Signale
					signal(SIGINT, SIG_IGN);
					signal(SIGQUIT, sigint_handler);
					
					//Signak CHLD auf IGN stellen sonst zählt er falsch
					signal(SIGCHLD,SIG_IGN);
					
					//warten das Childprozess fertig wird
					writetoscreen = 1;
					waitpid(child, &stat_loc, WUNTRACED);
					writetoscreen = 0;
				}
		}
		
		//Speicher freigeben und runBackground wieder auf 0 setzen
		free(cmdv);
		runBackground = 0;
	}
	//Return 0;
}

