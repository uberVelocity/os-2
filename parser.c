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


int validInputLine(char **inputLine, char **inputFilename, char **outputFilename) {
    // Check for final character being arrow.
    // Check that after arrow I have a name.
    int ioProblem = 0, i = 0, pipeProblem = 0, sameInOut = 0, backProblem = 0;
    while (inputLine[i] != NULL) {
        // Redirect problem
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
        // Pipe problem
        if (inputLine[i] != NULL && strcmp(inputLine[i], "|") == 0) {
            i++;
            if (inputLine[i] == NULL || (inputLine[i] != NULL && !isalnum(inputLine[i][0]))) {
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
int launch(char **args, char *inputFilename, char *outputFilename) {
    pid_t pid, wpid;
    char str1[10];
    int in, out, status, i = 0, j = 0, k = 0;
    char **pargs = calloc(TOK_BUFSIZE, sizeof(char*));
    // while (args[i] != NULL) {
    //     printf("args[%d] = %s\n", i, args[i]);
    //     i++;
    // }
    i = 0;
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
        // Remove quotes from argument
        while (pargs[k] != NULL) {
            // printf("BEFORE pargs%d = %s\n", k, pargs[k]);
            remove_all_chars(pargs[k], '"');
            // printf("AFTER pargs%d = %s\n", k, pargs[k]);
            k++;
        }	

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
            // printf("line[%d] = %c\n", i, line[i]);
            i++;
        }
        fqPointer = i;
        //printf("FIRST = %d\n", fqPointer);
        
        i = strlen(line) - 1;
        while (line[i] != '"') {
            i--;
        }
        lqPointer = i;
        //printf("LAST = %d\n", lqPointer);
        i = 0, pos = 0;
        // Populate tokens
        //printf("fqPointer = %d\nlqPointer = %d\n", fqPointer, lqPointer);
        for (i = 0; i < strlen(line); i++) {
            // Out of the largest quotation block
            if (i < fqPointer || i > lqPointer) {
                //printf("addtotoken: i = %d\n", i);
                // Spaces indicate arguments
                if (line[i] == ' ') {
                    printf("incrementing POS for i = %d, current pos = %d\n", i, pos);
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
                    if (tokens[pos][j] != 10) {
                        printf("tokens[%d][%d] = %c\n", pos, j, line[i]);
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
                printf("tokens[%d][%d] = %c\n", pos, j, line[i]);
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
            // printf("tokens[%d] = %s\n", pos, tokens[pos]);
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

char **splitLine(char* line) {
	int bufSize = TOK_BUFSIZE, pos = 0;
	int inQuotes = 0;
    char **tokens = calloc(bufSize, sizeof(char*));
	char *token;
	int numOfQuotes = 0;
	
	if (!tokens) {
		fprintf(stderr, "tokbuf: allocation error\n");
		exit(EXIT_FAILURE);
	}
    token = strtok(line, TOK_DELIM);
	while (token != NULL) {
        // Handle quotes.
        // printf("TBMtoken = %s\n", token);
        // Handle '"' character that is not just one word.
        
        // Handle Background processes.
        if (token[strlen(token) - 1] == '&' || strcmp(token, "&") == 0) {
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

char **splitCommands(char *line, int *numberCommands) {
    char **commands = calloc(TOK_BUFSIZE, sizeof(char*));
    char *delimiter = strtok(line, "|");
    int i = 0;
    while (delimiter != NULL) {
        commands[i] = delimiter;
        delimiter = strtok(NULL, "|");
        printf("commands[%d] = %s\n", i, commands[i]);
        i++;
    }
    *numberCommands = i;
    return commands;
}

void shLoop(void) {
    // init();
    char *line, *cpline;
    char **sepArgs; // Arguments separated by pipe symbol '|'.
    char **args;
    char **commands;
    int status = 1;
    int looped = 0;
    int numberCommands = 0, i = 0, j = 0;
    // Configure readline to auto-complete paths when the tab key is hit.
    // rl_bind_key('\t', rl_complete);
	do {
        char *inputFilename = 0, *outputFilename = 0;        
        // printf("looped = %d\n", looped);
        line = NULL;
        args = NULL;
        // Read input from user.
        line = readLine();
        cpline = strdup(line);  // Needed to validify the input without changing it.
        if (validInputLine(divideLine(cpline), &inputFilename, &outputFilename)) {
            commands = splitCommands(line, &numberCommands);
            i = 0;
            while (commands[i] != NULL) {
                args = divideLine(commands[i]);
                j = 0;
                while (args[j] != NULL) {
                    j++;
                }
                // status = execute(args, inputFilename, outputFilename, i, numberCommands);
                i++;
            }   
        }
        // Divide input into separate commands.
        
        
        // Use up arrow to retrieve command from history.
        // if (line && *line) {
        //     add_history(line);
        // }
        // int i = 0;
        // while (args[i] != NULL) {
        //     printf("token[%d] = %s\n", i, args[i]);
        //     i++;
        // }
        // Background process has been launched, ignore execution.
        
        
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
