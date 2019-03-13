#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ex1.h"


char *builtin_str[] = {
	"cd",
	"help",
	"exit"
};

int isFile(char* filename) {
	if (access(filename, F_OK ) != -1) {
    return 1;
  }
	return 0;
}

int isBuiltinCommand(char *string) {
		if ((strcmp(string, "cd")) == 0 || (strcmp(string, "help") == 0) || (strcmp(string, "exit") == 0)) {
			return 1;
		}
		return 0;
}

int num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}

int isValidCommand(char* string) {
    char *ps = getenv("PATH");
    char *parse = strtok(ps,":");
    char buffer[300][300];
    int index = -1, i = 0;
    printf("BUFFER = %s\n", buffer[0]);
    while (parse != NULL) {
        parse = strtok(NULL,":");
        if (parse != NULL) {
                strcpy(buffer[i], parse);
                strcat(buffer[i],"/");
                strcat(buffer[i], string);
        }
        i++;
    }	
    for(int j = 0; j < i - 1; j++) {
        if (!access(buffer[j], X_OK)) {
                index = j;
        }
    }
    ps = 0;
    parse = 0;
    for (i = 0; i < 300; i++) {
        for (int j = 0; j < 300; j++) {
            buffer[i][j] = 0;
        }
    }
    if (index == -1) {
        return 0;
	}
	return 1;
}

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
		// printf("token = %s\n", token);		
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




int launch(char **args, int mode) {
    pid_t pid, wpid;
    int in, out, status, i = 0, j = 0;

    pid = fork();
    if (pid == 0) {
        while (args[i] != NULL) {
            i++;
        }
        char **tokens = calloc(i + 1, sizeof(char*));
        i = 0;
        /**
         * Bread and butter. Should be refactored into functions for lizibility and modularity.
         * Test cases need to be thought of.
         */
        while (args[i] != NULL) {
            if (args[i][0] == '<') {
                if (args[i + 1] != NULL && args[i + 2] != NULL && args[i + 2][0] == '>') {
                    in = open(args[i + 1], O_RDONLY);
                    out = open(args[i + 3], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                    if (in != -1 && out != -1) {
											dup2(in, 0);
											dup2(out, 1);
											close(in);
											close(out);
										}
										else {
											printf("--- FILES DO NOT EXIST ---\n");
											break;
										}
                }
                else {
                    in = open(args[i + 1], O_RDONLY);
                    printf("in = %d\n", in);
                    if (in != -1) {
											dup2(in, 0);
											close(in);
											tokens[i] = NULL;
											break;
										}
										else {
											printf("--- FILES DO NOT EXIST ---\n");
											break;
										}
                    
                }
                         
            }
            if (args[i][0] == '>') {
                out = open(args[i + 1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
								dup2(out, 1);
                close(out);
                tokens[i] = NULL;
                break;  
								
            }
            tokens[i] = calloc(strlen(args[i]), sizeof(char));
            strcpy(tokens[i], args[i]);
            i++;
        }
        if (execvp(tokens[0], tokens) == -1) {
            printf("SHOULD NOT PRINT THIS!\n");
            perror("lsh");
            exit(EXIT_FAILURE);
        }
        else if (pid < 0) {
            // Error forking
            perror("lsh");
        }
        else {
            // wait(NULL);
            do {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }
    // printf("RETURNING 1\n");
    return 1;
    
}

int execute(char **args) {
    int i;

    if (args[0] == NULL) {
        return 1;
    }

    /**
     * Veify if input command is a builtin Linux command.
     */
    for (i = 0; i < num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0){
            return (*builtin_func[i])(args);
        }
    }
    return launch(args, REGULAR);
}
/*
char **pipeSep(char *line) {
	char *pchar;
	char *sepCmd[256];
	pchar = strtok(line, "|");
	int count = 0;
	while (pch != NULL && count < 256) {
      printf("%s\n", pch);
      sepCmd[count] = pch;
      printf("The value in this array value is: %s\n", sepCmd[count]);
      pch = strtok (NULL, "|");
      count++;
  }
  return sepCmd;
}
*/
void shLoop(void) {
	char *line;
    char **sepArgs; // Arguments separated by pipe symbol '|'.
    char **args;
    int status = 1;
    int looped = 0;
	char shell_prompt[100];
    // Configure readline to auto-complete paths when the tab key is hit.
    rl_bind_key('\t', rl_complete);
	do {
		// printf("> ");
        // line = readLine();
        line = NULL;
        args = NULL;
        line = readline("> ");
        //sepArgs = pipeSep(line);
        // Use up arrow to retrieve command from history.
        if (line && *line) {
            add_history(line);
        }
        /*char *pchar;
				char *sepCmd[256];
				pchar = strtok(line, "|");
				int count = 0;
				while (pchar != NULL && count < 256) {
						// printf("%s\n", pchar);
						sepCmd[count] = pchar;
						printf("The value in this array value is: %s\n", sepCmd[count]);
						pchar = strtok (NULL, "|");
						count++;
				}*/
				//if (count == 1) {
					args = splitLine(line);
					status = execute(args);
				//}
				//else if (count > 1) {
				//	printf("Handling multiple commands...\n");
				//}
				//else {
				//	printf("ERROR: should not have n = %d commands!\n", count);
					//exit(EXIT_FAILURE);
				//}
        // printf("status = %d\n", status);
        // abort();
        free(line);
        free(args);
	} while(status);
}


int main(int argc, char* argv[]) {
    setbuf(stdout, NULL);
    shLoop();
}
