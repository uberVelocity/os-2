#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

/*get args function*/

#define MAXARGS 256
char ** getargs(char * cmd) {
	printf("\ncmd = %s\n", cmd);
	// assumes that cmd ends with NULL
	char** argsarray;
	int nargs = 0;
	int nlen = strlen(cmd);
	int i = 0;
	argsarray = (char**) malloc(sizeof(char*) * MAXARGS);
	argsarray[0] = strtok(cmd," ");
	i = 0;
	while (argsarray[i] != NULL){
		i++;
		argsarray[i] = strtok(NULL," ");
	}
	return argsarray;
}


int main(void){

  pid_t childpid;
  int fd[256][2];
  char cmd[256];
  char * sepCmd[256];
  char * pch;

  printf("Please enter a command sequence: \n");
  gets(cmd);
  //scanf("%s", cmd);
  printf("You have entered: %s \n", cmd);
  

  printf("Attempting to split up command: \n");
  pch = strtok (cmd, "|");
  

  /* My initial idea of how to tackle redirection
  char * srch;
  srch = strchr(cmd, '>');
  int reDir;

  while(cmd != NULL) {
  if (srch != NULL){
    printf("redirect operator found\n");
    reDir = 1;
          
  }
  srch = strchr(srch+1, '>');
  }
  */
  
  
  
  int count = 0;  
    while (pch != NULL && count < 256) {
      printf("%s\n", pch);
      sepCmd[count] = pch;
      printf("The value in this array value is: %s\n", sepCmd[count]);
      pch = strtok (NULL, "|");
      count++;
  }
  
  char ** argue;
  int k;
  
  /* Block that deals with the first command given by the user */
  k = 0;
  pipe(fd[k]);
  if(!fork()) {
	  	dup2(fd[k][1], STDOUT_FILENO);
		close(fd[k][0]);
		argue = getargs(sepCmd[k]);
		execvp(argue[0], argue);
		perror(argue[0]);
		exit(0);
  }

  /*Loop that will control all other comands except the last*/
  for(k = 1; k <= count - 2; k++) {
	  close(fd[k-1][1]);
	  pipe(fd[k]);
	  
	  if(!fork()) {
		  close(fd[k][0]);
		  dup2(fd[k-1][0], STDIN_FILENO);
		  dup2(fd[k][1], STDOUT_FILENO);
		  argue = getargs(sepCmd[k]);
		  execvp(argue[0], argue);
		  perror(argue[0]);
		  exit(0);
	  }
  }
  
  
  /*Block that will take care of the last command in the sequence*/
  k = count - 1;
  
  //  if(reDir){
  //argue = getargs(sepCmd[k]);
  //open(argue[count], O_RDWR);
    
  //if(!fork()){
  //  close(fd[0]);
  //  close(fd[1]);
  //  dup2(fd[k-1][0], STDOUT_FILENO);
  //  execl("/bin/>", argue[count]);}
  //}
  //else{
     
  close(fd[k-1][1]);
  if(!fork()) {
	  dup2(fd[k-1][0], STDIN_FILENO);
	  argue = getargs(sepCmd[k]);
	  execvp(argue[0], argue);
	  perror(argue[0]);
	  exit(0);
  }
  // }
  while(waitpid(-1, NULL, 0) != -1);
}
