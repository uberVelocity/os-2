#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1024
#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"

int num_builtins(void);
int sh_cd(char **args);
int sh_help(char **args);
int sh_exit(char **args);
int launch(char**);
int execute(char**);
char **splitLine(char*);
char *readLine(void);
void shLoop(void);

char *builtin_str[] = {
	"cd",
	"help",
	"exit"
};

int (*builtin_func[]) (char **) = {
	&sh_cd,
	sh_help,
	sh_exit
};

int sh_cd(char **args) {
	if (args[1] == NULL) {
		fprintf(stderr, "lsh: expected argument to \"cd\"\n");
	}
	else {
		if (chdir(args[1]) != 0) {
			perror("lsh");
		}
	}
	return 1;
}

int sh_help(char **args) {
	int i;
	printf("Mihai Popescu's LSH\n");
	printf("Type program names and arguments, and hit enter.\n");
	printf("The following are built in:\n");
	
	for (i = 0; i < num_builtins(); i++) {
		printf("  %s\n", builtin_str[i]);
	}
	
	printf("Use the man command for information on other programs.\n");
	return 1;
}

int sh_exit(char **args) {
	return 0;
}

int num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}

void shLoop(void) {
	char *line;
	char **args;
	int status;
	
	do {
		printf("> ");
		line = readLine();
		args = splitLine(line);
		status = execute(args);
		
		free(line);
		free(args);
	} while(status);
}

char *readLine(void) {
	char *line = NULL;
	ssize_t bufSize = 0;
	getline(&line, &bufSize, stdin);
	return line;
}

char **splitLine(char* line) {
	int bufSize = TOK_BUFSIZE, pos = 0;
	char **tokens = calloc(bufSize, sizeof(char*));
	char *token;
	
	if (!tokens) {
		fprintf(stderr, "tokbuf: allocation error\n");
		exit(EXIT_FAILURE);
	}
	
	token = strtok(line, TOK_DELIM);
	while (token != NULL) {
			tokens[pos] = token;
			pos++;
			
			if (pos >= bufSize) {
				bufSize += TOK_BUFSIZE;
				tokens = realloc(tokens,bufSize * sizeof(char*));
				if (!tokens) {
					fprintf(stderr, "tokens: allocation error\n");
					exit(EXIT_FAILURE);
				}
			}
			
			token = strtok(NULL, TOK_DELIM);
		}
		tokens[pos] = NULL;
		return tokens;
	}

int launch(char **args) {
		pid_t pid, wpid;
		int status;
		
		pid = fork();
		if (pid == 0) {
			// Child process
			if (execvp(args[0], args) == -1) {
				perror("lsh");
				exit(EXIT_FAILURE); // SHOULD BE MOVED 80
			}
			else if (pid < 0) {
				// Error forking
				perror("lsh");
			}
			else {
				do {
					wpid = waitpid(pid, &status, WUNTRACED);
				} while (!WIFEXITED(status) && !WIFSIGNALED(status));
			}
		}
		return 1;
}

int execute(char **args) {
	int i;
	
	if (args[0] == NULL) {
		return 1;
	}
	
	for (i = 0; i < num_builtins(); i++) {
		if (strcmp(args[0], builtin_str[i]) == 0 ){
			return (*builtin_func[i])(args);
		}
	}
	return launch(args);
}

int main(int argc, char* argv[]) {
  	
	shLoop();
	
}
