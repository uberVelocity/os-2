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

int backgroundSymbol(char *token) {
    int i = 0;
    if (token[i] == '\0')   return 0;
    while (token[i] != '\0') {
        if (token[i] != '&') {
            return 0;
        }
        i++;
    }
    return 1;
}

int onlyMetacharacters(char *token) {
    int i = 0;
    if (token != NULL && token[i] == '\0')   return 0;
    while (token != NULL && token[i] != '\0') {
        if (token[i] != '&' && token[i] != '<' && token[i] != '>' && token[i] != '|') {
            return 0;
        }
        i++;
    }
    return 1;
}

/**
 * Valid when all arguments are non-empty, second argument is a meta-character and fourth argument is background
 * character.
 */ 
int validC4(char **tokens) {
    return 0;
}

/**
 * Valid when first three arguments are non-empty, fourth argument is empty and
 * second argument has only meta characters.
 */
int validC3(char **tokens) {
    printf("checking C3!\n");
    int counter = 0;
    if (tokens[0] == NULL) return 0;    
    int i = 0;
    while (tokens[i + 1] != NULL) {
        if (tokens[i][1] == 0 && (tokens[i][0] == '<' || tokens[i][0] == '>' || tokens[i][0] == '|') && tokens[i-1] != NULL
        && tokens[i-1][0] != '<' && tokens[i-1][0] != '>' && tokens[i-1][0] != '|' && tokens[i-1][0] != '&'
        && tokens[i+1] != NULL
        && tokens[i+1][0] != '<' && tokens[i+1][0] != '>' && tokens[i+1][0] != '|' && tokens[i+1][0] != '&') {
            printf("TOKEN = %c\n", tokens[i][0]);
            counter++;
        }
        i++;
    }
    if (tokens[i][0] == '&') {
        printf("C3 - [ ]\n");
        return 0;
    }
    if (counter == 1) {
        printf("C3 - [X]\n");
        return 1;
    }
    printf("C3 - [ ]\n");
    return 0;
}

/**
 * Valid when tokens[0] is non-empty, tokens[1] is a meta character
 * and tokens[2] and tokens[3] are empty.
 */
int validC2(char **tokens) {
    printf("checking C2!\n");
    if (tokens[0] == NULL) return 0;
    int i = 0;    
    while (tokens[i + 1] != NULL) {
        if (tokens[i][0] == '>' || tokens[i][0] == '<' || tokens[i][0] == '|') {
            return 0;
        }
        i++;
    }
    if (tokens[i][0] == '&') {
        printf("C2 - [X]\n");
        return 1;
    }
    printf("C2 - [ ]\n");
    return 0;
}

int hasSpecialCharacter(char **tokens) {
    int i = 0;
    while (tokens[i] != NULL) {
        if (tokens[i][0] == '>' || tokens[i][0] == '<' || tokens[i][0] == '|' || tokens[i][0] == '&') {
            return 1;
        }
        i++;
    }
    return 0;
}

/**
 * Valid when everything except tokens[1] is empty.
 */
int validC1(char **tokens) {
    printf("checking C1!\n");
    if (hasSpecialCharacter(tokens)) {
        printf("C1 - [ ]\n");
        return 0;
    }
    printf("C1 - [X]\n");
    return 1;
}

int launchC3(char **tokens) {
    int i = 0, j = 0;
    while (tokens[i] != NULL) {
        if (strcmp(tokens[i], "<") == 0) {
            launch(tokens, INPUT);
            char **args = calloc(i + 1, sizeof(char*));
            for (j = 0; j < i; j++) {
                args[j] = calloc(strlen(tokens[j]), sizeof(char));
                strcpy(args[j], tokens[j]);
            }
            args[i] = NULL;
            for (j = 0; j < i; j++) {
                printf("original = %s\n new = %s\n\n", tokens[j], args[j]);
            }
            int in, out, status;
            pid_t pid, wpid;
            printf("TOKENS[I + 1] ==== %s\n", tokens[i + 1]);
            in = open(tokens[i + 1], O_RDONLY);
            dup2(in, 0);
            close(in);
            execute(args);
            // pid = fork();
            // if (pid == 0) {
            //     // Child process
            //     if (execvp(args[0], args) == -1) {
            //         perror("lsh");
            //         exit(EXIT_FAILURE);
            //     }
            //     else if (pid < 0) {
            //         // Error forking
            //         perror("lsh");
            //     }
            //     else {
            //         do {
            //             wpid = waitpid(pid, &status, WUNTRACED);
            //             printf("WPID = %d\n", wpid);
            //         } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            //     }
            // }
            // for (int j = 0; j < i; j++) {
            //     free(args[i]);
            // }
            // free(args);
        }
        i++;
    }
}

/**
 * It checks if the command is valid.
 * In the future, if it is valid, run corresponding
 * action for CN, where N = {1, 2, 3, 4}.
 */
int validCommand(char **tokens) {
    printf("IN VALID COMMAND!");
    if (validC4(tokens)) {
        // return launchC4(tokens);
    }
    if (validC3(tokens)) {
        return launchC3(tokens);
    }

    if (validC2(tokens)) {
        // launchC2(tokens);
        return 1;
    }
    if (validC1(tokens)) {
        if (tokens != NULL) launch(tokens, REGULAR);
    }
    // printf("INVALID COMMAND!\n");
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
/**
 * While args[i] != NULL
 * launchToken0 takes first argument until mode/background.
 * Mode takes special symbol.
 * launchToken1 takes second argument until mode/background.
 */
char **parseCommand2(char **args) {
    char **tokenlaunch1, **tokenlaunch2;
    int i = 0, j = 0;
    int mode;

    tokenlaunch1 = calloc(64, sizeof(char*));
    for (i = 0; i < 64; i++) {
        tokenlaunch1[i] = calloc(50, sizeof(char));
    }

    tokenlaunch2 = calloc(64, sizeof(char*));
    for (i = 0; i < 64; i++) {
        tokenlaunch2[i] = calloc(50, sizeof(char));
    }

    while (args[i] != NULL) {
        //TODO: UPDATE LIST
        while (args[i][0] != '<' && args[i][0] != '>' && args[i][0] != '|' && args[i][0]) {
            strcpy(tokenlaunch1[i], args[i]);
            i++;
        }
        if (args[i][0] == '<')  mode = INPUT;
        if (args[i][0] == '>')  mode = OUTPUT;
        if (args[i][0] == '|')  mode = PIPELINE;
        i++;
        while (args[i][0] != '<' && args[i][0] != '>' && args[i][0] != '|' && args[i][0]) {
            strcpy(tokenlaunch2[j], args[i]);
            i++;
        }            
    }
}


char **parseCommand(char **args){
    char *token_1, *token_2, *token_3, *token_4;
    int i = 0;
    
    token_1 = calloc(BUFFER_SIZE, sizeof(char));
    assert(token_1 != NULL);
    while (args[i] != NULL && args[i][0] != '<' && args[i][0] != '>' && args[i][0] != '&' && args[i][0] != '|') {
        strcat(token_1, args[i]);
        strcat(token_1, " ");
        i++;
    }
    printf("token_1 = %s\n", token_1);

    token_2 = calloc(BUFFER_SIZE, sizeof(char));
    assert(token_2 != NULL);
    while (args[i] != NULL && !isalnum(args[i][0])) {
        strcat(token_2, args[i]);
        i++;
    }
    printf("token_2 = %s\n", token_2);

    token_3 = calloc(BUFFER_SIZE, sizeof(char));
    assert(token_3 != NULL);
    while (args[i] != NULL && args[i][0] != '<' && args[i][0] != '>' && args[i][0] != '&') {
        strcat(token_3, args[i]);
        strcat(token_3, " ");
        i++;
    }
    printf("token_3 = %s\n", token_3);
    
    token_4 = calloc(BUFFER_SIZE, sizeof(char));
    assert(token_4 != NULL);
    while (args[i] != NULL && !isalnum(args[i][0])) {
        strcat(token_4, args[i]);
        i++;
    }
    printf("token_4 = %s\n", token_4);

    char **tokens = calloc(4, sizeof(char*));
    for (i = 0; i < 4; i++) {
        tokens[i] = calloc(50, sizeof(char));
    }

    strcpy(tokens[0], token_1);
    strcpy(tokens[1], token_2);
    strcpy(tokens[2], token_3);
    strcpy(tokens[3], token_4);

    if (tokens[0][0] != 0)  tokens[0][strlen(tokens[0]) - 1] = '\0';
    else tokens[0] = NULL;
    if (tokens[1][0] != 0)  {
        tokens[1][strlen(tokens[1]) - 1] = '\0';
        printf("token 1 is not null BITCH\n\n\n");
    }
    else tokens[1] = NULL;
    if (tokens[2][0] != 0)  tokens[2][strlen(tokens[2]) - 1] = '\0';
    else tokens[2] = NULL;
    if (tokens[3][0] != 0)  tokens[3][strlen(tokens[3]) - 1] = '\0';
    else tokens[3] = NULL;

    free(token_1);
    free(token_2);
    free(token_3);
    free(token_4);

    return tokens;   
}

int launch(char **args, int mode) {
    pid_t pid, wpid;
    int in, out, status, i = 0, j = 0;

    

    // if (mode == OUTPUT) {
    //     int in, out;
    //     out = open(args[2], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    //     printf("args 2 = %s\n", args[2]);
    //     dup2(out, 1);
    //     close(out);
    // }
    // if (mode == PIPELINE) {

    // }

    pid = fork();
    if (pid == 0) {
        while (args[i] != NULL) {
            i++;
        }
        char **tokens = calloc(i + 1, sizeof(char*));
        i = 0;
        while (args[i] != NULL) {
            printf("i = %d\n", i);
            if (args[i][0] == '<') {
                if (args[i + 1] != NULL && args[i + 2] != NULL && args[i + 2][0] == '>') {
                    in = open(args[i + 1], O_RDONLY);
                    out = open(args[i + 3], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                    dup2(in, 0);
                    dup2(out, 1);
                    close(in);
                    close(out);
                }
                else {
                    in = open(args[i + 1], O_RDONLY);
                    dup2(in, 0);
                    close(in);
                    tokens[i] = NULL;
                    printf("%s\n%s\n%s\n%s\n%s\n\n", args[0], args[1], args[2], args[3], args[4]);
                    printf("%s\n%s\n%s\n%s\n%s\n", tokens[0], tokens[1], tokens[2], tokens[3], tokens[4]);   
                    break;
                }
                         
            }
            if (args[i][0] == '>') {
                out = open(args[i + 1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                printf("args 2 = %s\n", args[2]);
                dup2(out, 1);
                close(out);
                printf("FILE DESCRIPTOR NAME = %s\n", args[i + 1]);
                tokens[i] = NULL;
                printf("%s\n%s\n%s\n%s\n%s\n\n", args[0], args[1], args[2], args[3], args[4]);
                printf("%s\n%s\n%s\n%s\n%s\n", tokens[0], tokens[1], tokens[2], tokens[3], tokens[4]);   
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
            wait(NULL);
            do {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }
    printf("RETURNING 1\n");
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


void shLoop(void) {
	char *line;
    char **args;
    int status = 1;
    int looped = 0;
	char shell_prompt[100];
    // Configure readline to auto-complete paths when the tab key is hit.
    // rl_bind_key('\t', rl_complete);
	do {
		// printf("> ");
        // line = readLine();
        line = NULL;
        args = NULL;
        line = readline("> ");
        
        // Use up arrow to retrieve command from history.
        if (line && *line) {
            add_history(line);
        }
        
        args = splitLine(line);
        
        //tokens = parseCommand(args);
        // printf("COMP 0:%d\n", strcmp(tokens[0], args[0]));
        printf("\n%s\n", args[0]);

        // status = validCommand(args);

        status = execute(args);
        printf("status = %d\n", status);
        // abort();
        free(line);
        free(args);
	} while(status);
}

void loop() {
    char *line = readLine();
    char **args = splitLine(line);
    char ** tokens = parseCommand(args);
    validCommand(tokens);
}

int main(int argc, char* argv[]) {
    setbuf(stdout, NULL);
    shLoop();
}