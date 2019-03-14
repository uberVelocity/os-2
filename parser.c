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
	printf("Mihai Popescu's and Alex Scurtu's LSH\n");
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

char *strdup (const char *s) {
    char *d = malloc (strlen(s) + 1);
    if (d == NULL)  return NULL;
    strcpy(d,s);
    return d;
}

void init() {
    printf("\n");
    printf("*     SHELL - CORP - 1.0      *\n");
    printf("*     -------------------     *\n");
    printf("*                             *\n");
    printf("*                             *\n");
    printf("*    AUTHORS: Mihai Popescu   *\n");
    printf("*             Andrei Scurtu   *\n");
    printf("\n\n");
}

void x2rectangle() {
    printf("\n*****************\n*\t\t*\n*\t\t*\n*\t\t*\n*****************\n");
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
        // Handle '"' character.
        if (token[0] == '"' && token[strlen(token) - 1] != '"') {
            int posp = pos;
            while (token != NULL && token[strlen(token) - 1] != '"') {
                // First token and it has quotes.  
                if (tokens[posp] == NULL) {
                    strcpy(token, &token[1]);
                    tokens[posp] = strdup(token);
                }
                // There have been other tokens, resize tokens at that position and concatenate the new one.
                else {
                    tokens[posp] = realloc(tokens[posp], (strlen(tokens[posp]) + strlen(token) + 2) * sizeof(char));
                    strcat(tokens[posp], " ");
                    strcat(tokens[posp], token);                    
                }
                token = strtok(NULL, TOK_DELIM);
                if (token[strlen(token) - 1] == '"') {
                    tokens[posp] = realloc(tokens[posp], (strlen(tokens[posp]) + strlen(token) + 2) * sizeof(char));
                    strcat(tokens[posp], " ");
                    token[strlen(token) - 1] = 0;
                    strcat(tokens[posp], token);
                    token = strtok(NULL, TOK_DELIM);
                }
                if (token == NULL) {
                    pos++;
                }
            }
        }
        else {
            // Handle background processes
            if (token[strlen(token) - 1] == '&' || strcmp(token, "&") == 0) {
								if (strcmp(token, "&") == 0)	printf("IN IF BECAUSE BACKGROUND\n");
                token[strlen(token) - 1] = 0;
                if (token != 0) {
                    tokens[pos] = token;                    
                }
                pos++;
                tokens[pos] = NULL;
                MODE = BACKGROUND;
                launch(tokens);
                free(tokens);
                tokens = calloc(bufSize, sizeof(char*));
                pos = 0;
                token = strtok(NULL, TOK_DELIM);
            }
            // Handle normal tokens
            if (token != NULL && strcmp(token, "&") != 0) {
                MODE = REGULAR;
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
            }
            token = strtok(NULL, TOK_DELIM);
        }
    }
    tokens[pos] = NULL;
	return tokens;
}




int launch(char **args) {
    pid_t pid, wpid;
    int in, out, status, i = 0, j = 0;
    pid = fork();
    if (pid == 0) {
        while (args[i] != NULL) {
            i++;
        }
        char **tokens = calloc(i + 2, sizeof(char*));
        i = 0;
        /**
         * Bread and butter. Should be refactored into functions for lizibility and modularity.
         * Test cases need to be thought of.
         */
        while (args[i] != NULL) {
            if (args[i][0] == '<') {
                if (args[i + 1] != NULL && args[i + 2] != NULL && args[i + 2][0] == '>') {
                    if (isFile(args[i + 1]) && isFile(args[i + 3])) {
											in = open(args[i + 1], O_RDONLY);
											out = open(args[i + 3], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
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
									if (isFile(args[i + 1])) {
												in = open(args[i + 1], O_RDONLY);
												printf("in = %d\n", in);
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
            printf("Command %s not found!\n", tokens[0]);
            exit(EXIT_FAILURE);
        }
    }
    else if (pid < 0) {
        // Error forking
        perror("lsh");
    }
    else {
        if (MODE != BACKGROUND) {
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
					printf("BUILT IN!\n");
            return (*builtin_func[i])(args);
        }
    }
    return launch(args);
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
    init();
		char *line;
    char **sepArgs; // Arguments separated by pipe symbol '|'.
    char **args;
    int status = 1;
    int looped = 0;
		char shell_prompt[100];
    // Configure readline to auto-complete paths when the tab key is hit.
    rl_bind_key('\t', rl_complete);
	do {
        // printf("looped = %d\n", looped);
        line = NULL;
        args = NULL;
        line = readline("> ");
        // Use up arrow to retrieve command from history.
        if (line && *line) {
            add_history(line);
        }

        args = splitLine(line);
        // Background process has been launched, ignore execution.

        status = execute(args);
        free(args);

        free(line);
        MODE = REGULAR;
        looped++;
	} while(status);
}


int main(int argc, char* argv[]) {
    setbuf(stdout, NULL);
    shLoop();
}
