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
	printf("The shell supports piping, redirection and string parsing.\n");
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
    printf("*     SHELL - OS -  v1.0      *\n");
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


int validInputLine(char **inputLine, char **inputFilename, char **outputFilename) {
    int ioProblem = 0, i = 0, pipeProblem = 0, sameInOut = 0, backProblem = 0;
    while (inputLine[i] != NULL) {
        // Redirect problem
        if (inputLine[i] != NULL && (strcmp(inputLine[i], "<") == 0 || strcmp(inputLine[i], ">") == 0)) {
            i++;
            if (inputLine[i] == NULL || (inputLine[i] != NULL && (!isalnum(inputLine[i][0]) && inputLine[i][0] != '.'))) {                
                ioProblem = 1;
            }
            if (inputLine[i] != NULL && (inputLine[i][0] == '"' || inputLine[i][0] == '.' || isalnum(inputLine[i][0])) && strcmp(inputLine[i-1], "<") == 0) {
                inputFilename[0] = strdup(inputLine[i]);
            }
            if (inputLine[i] != NULL && (inputLine[i][0] == '"' || inputLine[i][0] == '.' || isalnum(inputLine[i][0])) && strcmp(inputLine[i-1], ">") == 0) {
                outputFilename[0] = strdup(inputLine[i]);
            }
        }
        // Pipe problem
        if (inputLine[i] != NULL && strcmp(inputLine[i], "|") == 0) {
            i++;
            if (inputLine[i-2] == NULL || inputLine[i] == NULL || (inputLine[i] != NULL && !isalnum(inputLine[i][0]))) {
                pipeProblem = 1;
            }
        }
        // Background problem
        if (inputLine[i] != NULL && strcmp(inputLine[i], "&") == 0 && inputLine[i + 1] != NULL && !isalnum(inputLine[i + 1][0])) {
            printf("backProblem\n");
            backProblem = 1;
        }
        if (ioProblem || pipeProblem || backProblem) {
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

void remove_all_chars(char* str, char c) {
    char *pr = str, *pw = str;
    while (*pr) {
        *pw = *pr++;
        pw += (*pw != c);
    }
    *pw = '\0';
}

/**
 * Another function is required since this will do the input output for 
 * the current executed command, whereas input should only be done for 
 * the first command and output should only be done for the last command. 
 */
int launch(char **args, char *inputFilename, char *outputFilename, int nthCommand, int totalCommands, int *pipefds, int j) {
    pid_t pid, wpid;
    int in, out, status, i = 0, k = 0;
    i = 0;
    pid = fork();
    if (pid == 0) {
        // First command which needs to take input file if it exists.
        if (nthCommand == 0) {
            // If there is an in file, transfer stdio to it.
            if (inputFilename != NULL) {
                in = open(inputFilename, O_RDONLY);
                if (in == -1) {
                    perror(0);
                    exit(EXIT_FAILURE);
                }
                dup2(in, 0);
                close(in);
                
                
            }
            // Case where there is an out file and only one command. *CHECK THIS CONDITION*
            if (outputFilename != NULL && totalCommands == 1) {
                out = open(outputFilename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                if (out == -1) {
                    perror(0);
                    exit(EXIT_FAILURE);
                }
                dup2(out, 1);
                close(out);
            }
        }
        // Last command should redirect stdout to out if it exists.
        else if (nthCommand == totalCommands - 1) {
            if (outputFilename != NULL) {
                out = open(outputFilename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                if (out == -1) {
                    perror(0);
                    exit(EXIT_FAILURE);
                }
                dup2(out, 1);
                close(out);
            }
        }
        if (execvp(args[0], args) == -1) {
            printf("Error: command not found!\n");
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
    free(args);
    return 1;
}

char **divideLine(char *line) {
    int bufSize = TOK_BUFSIZE, i = 0, j = 0, quotes = 0;;
    char **tokens = calloc(bufSize, sizeof(char*));
    char *token;

    int pos = 0;
    int fqPointer = 0, lqPointer = strlen(line) - 1;

    if (!tokens) {
        fprintf(stderr, "tokbuf: allocation error\n");
		exit(EXIT_FAILURE);
	}

    for (i = 0; i < bufSize; i++) {
        tokens[i] = calloc(bufSize, sizeof(char));
    }


    // Check if input has quotes
    for (i = 0; i < strlen(line); i++) {
        if (line[i] == '"') {
            quotes = 1;
        }
    }
    if (quotes) {
        // Detect first and last quotation mark
        i = 0;
        while (line[i] != '"') {
            i++;
        }
        fqPointer = i;
        
        i = strlen(line) - 1;
        while (line[i] != '"') {
            i--;
        }
        lqPointer = i;
        i = 0, pos = 0;
        // Populate tokens
        for (i = 0; i < strlen(line); i++) {
            // Out of the largest quotation block
            if (i < fqPointer || i > lqPointer) {
                // Spaces indicate arguments
                if (i != 0 && line[i] == ' ') {
                    pos++;
                    j = 0;
                    if (pos >= bufSize) {
                    bufSize += TOK_BUFSIZE;
                    tokens = realloc(tokens,bufSize * sizeof(char*));
                        if (!tokens) {
                            fprintf(stderr, "tokens: allocation error\n");
                            exit(EXIT_FAILURE);
                        }
                    }
                }
                // Add character to token at position pos
                else {
                    if (tokens[pos][j] != 10 && tokens[pos][j] != ' ') {
                        tokens[pos][j] = line[i];
                        j++;
                    }
                }
            }
            else if ((i == fqPointer || i == lqPointer) && (line[i-1] != ' ' && line[i-1] != '\n' && (line[i+1] != ' ' && line[i+1] != 10 && line[i+1] != 0))) {
                pos++;
                j = 0;
                if (pos >= bufSize) {
                    bufSize += TOK_BUFSIZE;
                    tokens = realloc(tokens,bufSize * sizeof(char*));
                    if (!tokens) {
                        fprintf(stderr, "tokens: allocation error\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }
            else {
                if (i != fqPointer && i != lqPointer) {
                    if (tokens[pos][j] != 10) {
                        tokens[pos][j] = line[i];
                        j++;
                    }
                    
                }
            }
            // Inside the largest quotation block
            
        }
        tokens[pos + 1] = NULL;
    }
    else {
        token = strtok(line, TOK_DELIM);
        pos = 0;
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
    }
    if (tokens[pos] != NULL && tokens[pos][strlen(tokens[pos]) - 1] == '\n')  tokens[pos][strlen(tokens[pos]) - 1] = 0; 
    return tokens;

}

char **splitCommands(char *line, int *numberCommands) {
    char **commands = calloc(TOK_BUFSIZE, sizeof(char*));
    char *delimiter = strtok(line, "|");
    int i = 0;
    while (delimiter != NULL) {
        commands[i] = delimiter;
        delimiter = strtok(NULL, "|");
        i++;
    }
    *numberCommands = i;
    return commands;
}

char **removeIO(char **args) {
    char **newArgs = calloc(TOK_BUFSIZE, sizeof(char*));
    int i = 0, j = 0;
    i = 0;
    while (args[i] != NULL) {
        if (strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0) {
            i++;
        }
        else {
            newArgs[j] = strdup(args[i]);
            j++;
        }
        i++;
    }
    i = 0;
    return newArgs;
}

int execute(char **args, char *inputFilename, char *outputFilename, int nthCommand, int totalCommands, int *pipefds, int j) {
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
    return launch(args, inputFilename, outputFilename, nthCommand, totalCommands, pipefds, j);
}


void shLoop(void) {
    init();
    pid_t pid;
    char *line, *cpline;
    char **sepArgs; // Arguments separated by pipe symbol '|'.
    char **args;
    char **commands;
    char **executionCommand;
    int *pipefds;
    int status, isValid = 0;
    int looped = 0;
    int in, out;
    int numberCommands = 0, i = 0, j = 0;
    // Configure readline to auto-complete paths when the tab key is hit.
    // rl_bind_key('\t', rl_complete);
	do {
        char *inputFilename = 0, *outputFilename = 0;        
        // printf("looped = %d\n", looped);
        line = NULL;
        args = NULL;
        // Read input from user.
        printf("> ");
        line = readLine();
        line[strlen(line) - 1] = 0;
        cpline = strdup(line);  // Needed to validify the input without changing it.
        if (validInputLine(divideLine(cpline), &inputFilename, &outputFilename)) {
            i = 0;
            int k = 0;
            commands = splitCommands(line, &numberCommands);
            // Fire single command, do not handle piping.
            if (numberCommands == 1) {
                i = 0;
                while (commands[i] != NULL) {
                    args = divideLine(commands[i]);
                    // Remove front space from command argument
                    if (args[0][0] == ' ') {
                        memmove(&args[0][0], &args[0][1], strlen(args[0]) - 0);
                    }
                    executionCommand = removeIO(args);
                    status = execute(executionCommand, inputFilename, outputFilename, i, numberCommands, pipefds, j);
                    i++;
                }
            }
            // Handle multiple commands -> pipes!
            else {
                i = 0;

                while(commands[i] != NULL) {
                    isValid = validInputLine(commands, &inputFilename, &outputFilename);
                    i++;
                }
                if (isValid) {
                    pipefds = calloc(2 * numberCommands, sizeof(int));
                    for (i = 0; i < numberCommands; i++) {
                        if (pipe(pipefds + i*2) < 0) {
                            perror("couldn't pipe");
                            exit(EXIT_FAILURE);
                        }
                    }
                    j = 0;
                    k = 0;
                    while (commands[k] != NULL) {
                        pid = fork();
                        if (pid == 0) {
                            if (k == 0) {
                                if (inputFilename != NULL) {
                                    in = open(inputFilename, O_RDONLY);
                                    dup2(in, 0);
                                    close(in);
                                }
                            }
                            if (k != 0 && commands[k + 1] != NULL) {
                                if (dup2(pipefds[j + 1], 1) < 0) {
                                    perror("dup2");
                                    exit(EXIT_FAILURE);
                                }
                            }
                            if (j != 0) {
                                if(dup2(pipefds[j-2], 0) < 0) {
                                    perror("dup2");
                                    exit(EXIT_FAILURE);
                                }
                            }
                            for (i = 0; i < 2 * numberCommands; i++) {
                                close(pipefds[i]);
                            }
                            args = divideLine(commands[k]);
                            if (args[0][0] == ' ') {
                                memmove(&args[0][0], &args[0][1], strlen(args[0]) - 0);
                            }
                            executionCommand = removeIO(args);
                            if (execvp(executionCommand[0], executionCommand) < 0) {
                                perror(executionCommand[0]);
                                exit(EXIT_FAILURE);
                            }
                        }
                        else if (pid < 0) {
                            perror("error");
                            exit(EXIT_FAILURE);
                        }
                        k++;
                        j+=2;
                    }
                    free(pipefds);
                }
                
            }
            if (numberCommands > 1) {
                for (i = 0; i < 2 * numberCommands; i++) {
                    close(pipefds[i]);
                }

                for(i = 0; i < numberCommands + 1; i++) {
                    wait(&status);
                }
            }
        }
        free(commands);
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
