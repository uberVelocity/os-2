#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
// #include <readline/readline.h>
// #include <readline/history.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

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
	int numOfQuotes = 0;
	
	if (!tokens) {
		fprintf(stderr, "tokbuf: allocation error\n");
		exit(EXIT_FAILURE);
	}
    token = strtok(line, TOK_DELIM);
	while (token != NULL) {
			// printf("token = %s\n", token);
        // Handle '"' character that is not just one word.
        if (token[0] == '"' && token[strlen(token) - 1] != '"') {
            int posp = pos;
            while (token != NULL && token[strlen(token) - 1] != '"') {
								// printf("token inner while = %s\n", token);
                // First token and it has quotes.  
                if (tokens[posp] == NULL) {
										token++;                    
                    tokens[posp] = strdup(token);
                }
                // There have been other tokens, resize tokens at that position and concatenate the new one.
                else {
										if (token[0] == '"') {
											//printf("token to remove first char = %s\n", token);
											token++;
										}
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
        // Handle Background processes.
        else if (token[strlen(token) - 1] == '&' || strcmp(token, "&") == 0) {
            //if (strcmp(token, "&") == 0)	printf("IN IF BECAUSE BACKGROUND\n");
            token[strlen(token) - 1] = 0;
            if (token != 0) {
                tokens[pos] = token;          
                         
            }
            pos++; 
            tokens[pos] = NULL;
            MODE = BACKGROUND;
            // REMOVE NULLS - HAVE TO HANDLE THIS DIFFERENTLY ANYWAY
            launch(tokens, NULL, NULL);
            free(tokens);
            tokens = calloc(bufSize, sizeof(char*));
            pos = 0;
            token = strtok(NULL, TOK_DELIM);
        }
        // Handle normal processes.
        else if (token != NULL && strcmp(token, "&") != 0) {
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
    tokens[pos] = NULL;
	return tokens;
}

int validInputLine(char **inputLine, char **inputFilename, char **outputFilename) {
    // Check for final character being arrow.
    // Check that after arrow I have a name.
    int ioProblem = 0, i = 0, pipeProblem = 0, sameInOut = 0;
    while (inputLine[i] != NULL) {
        if (inputLine[i] != NULL && (strcmp(inputLine[i], "<") == 0 || strcmp(inputLine[i], ">") == 0)) {
            i++;
            if (inputLine[i] == NULL || (inputLine[i] != NULL && !isalnum(inputLine[i][0]))) {                
                ioProblem = 1;
            }
            if (inputLine[i] != NULL && isalnum(inputLine[i][0]) && strcmp(inputLine[i-1], "<") == 0) {
                inputFilename[0] = strdup(inputLine[i]);
            }
            if (inputLine[i] != NULL && isalnum(inputLine[i][0]) && strcmp(inputLine[i-1], ">") == 0) {
                outputFilename[0] = strdup(inputLine[i]);
            }
        }
        if (inputLine[i] != NULL && strcmp(inputLine[i], "|") == 0) {
            i++;
            if (inputLine[i] == NULL || (inputLine[i] != NULL && !isalnum(inputLine[i][0]))) {
                pipeProblem = 1;
            }
        }
        if (ioProblem || pipeProblem) {
            printf("Invalid syntax!\n");
            return 0;   
        }
        if (inputFilename[0] != NULL && outputFilename[0] != NULL) {
            if (strcmp(inputFilename[0], outputFilename[0]) == 0) {
            printf("Error: input and output files cannot be equal!\n");
            return 0;
            } 
        }
        i++;
    }
    return 1;
}

char* replace_char(char* str, char find, char replace){
    char *current_pos = strchr(str,find);
    while (current_pos){
        *current_pos = replace;
        current_pos = strchr(current_pos,find);
    }
    return str;
}

/**
 * Another function is required since this will do the input output for 
 * the current executed command, whereas input should only be done for 
 * the first command and output should only be done for the last command. 
 */
int launch(char **args, char *inputFilename, char *outputFilename) {
    pid_t pid, wpid;
    int in, out, status, i = 0, j = 0, stop = 0;
    char **pargs = calloc(TOK_BUFSIZE, sizeof(char*));
    pid = fork();
    if (pid == 0) {
        // printf("\nin:%s\nout:%s\n", inputFilename, outputFilename);
        while (args[i] != NULL) {
						if (strcmp(args[i], "<") != 0 && strcmp(args[i], ">") != 0) {
								pargs[j] = strdup(args[i]);
								i++;
								j++;
						}
						else {
								i+= 2;
						}
				}
				// pargs[i -j + 1] = NULL;
				i = 0;
				int k = 1;
				while (pargs[i] != NULL) {
					for (j = 0; j < strlen(pargs[i] - 1); j++) {
								if (pargs[i][j] == '"') {
									for (k = j+1; k < strlen(pargs[i] - 1); k++) {
										pargs[i][j] = pargs[i][k];
									}
								}
						}
					
					printf("pargs after dup and clean string: %s\n", pargs[i]);
					i++;
				}
				
        // Could potentially fix background prosesses with & separated.
        /*while (args[i] != NULL) {
            if (args[i+1][0] == 0) {
                args[i+1][0] = 0;
            }
            printf("args[%d][0] = %d\n", i, args[i][0]);
            i++;
        }*/
        if (inputFilename != NULL && outputFilename != NULL) {
            // VERIFY THAT THEY EXIST!
            in = open(inputFilename, O_RDONLY);
            out = open(outputFilename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
            dup2(in, 0);
            dup2(out, 1);
            close(in);
            close(out);
        }
        else if (inputFilename != NULL && outputFilename == NULL) {
            in = open(inputFilename, O_RDONLY);
            dup2(in, 0);
            close(in);
        }
        else if (inputFilename == NULL && outputFilename != NULL) {
            out = open(outputFilename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
            dup2(out, 1);
            close(out);
        }
        if (execvp(pargs[0], pargs) == -1) {
            printf("Error: command not found!\n");// , args[0]);
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
    free(pargs);
    // printf("RETURNING 1\n");
    return 1;
}

int execute(char **args, char *inputFilename, char *outputFilename) {
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
    return launch(args, inputFilename, outputFilename);
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
    // init();
    char *line;
    char **sepArgs; // Arguments separated by pipe symbol '|'.
    char **args;
    int status = 1;
    int looped = 0;
    // Configure readline to auto-complete paths when the tab key is hit.
    // rl_bind_key('\t', rl_complete);
	do {
        char *inputFilename = 0, *outputFilename = 0;        
        // printf("looped = %d\n", looped);
        line = NULL;
        args = NULL;
        line = readLine();
        // Use up arrow to retrieve command from history.
        // if (line && *line) {
        //     add_history(line);
        // }
        args = splitLine(line);
        int i = 0;
        // while (args[i] != NULL) {
        //  printf("token[%d] = %s\n", i, args[i]);
        //  i++;
        // }
        // Background process has been launched, ignore execution.
        if (validInputLine(args, &inputFilename, &outputFilename)) {
            status = execute(args, inputFilename, outputFilename);
        }
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
