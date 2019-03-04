#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "parser.h"

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
    if (token[i] == '\0')   return 0;
    while (token[i] != '\0') {
        if (token[i] != '&' && token[i] != '<' && token[i] != '>' && token[i] != '|') {
            return 0;
        }
        i++;
    }
    return 1;
}

int validC4(char **tokens) {

}

/**
 * Valid when first three arguments are non-empty, fourth argument is empty and
 * second argument has only meta characters.
 */
int validC3(char **tokens) {
    printf("testing C3\n");
    if (tokens[0][0] != '\0' && onlyMetacharacters(tokens[1]) && tokens[2][0] != '\0' && tokens[3][0] == '\0') {
        printf("C3 - [x]\n");
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
    printf("testing C2\n");
    if (tokens[0][0] != '\0' && backgroundSymbol(tokens[1])) {
        printf("C2 - [X]\n");
        return 1;
    }
    printf("C2 - [ ]\n");        
    return 0;
}

/**
 * Valid when everything except tokens[1] is empty.
 */
int validC1(char **tokens) {
    printf("testing C1\n");
    if (tokens[0][0] != '\0' && tokens[1][0] == '\0' && tokens[2][0] == '\0' && tokens[3][0] == '\0') {
        printf("C1 - [X]\n");
        return 1;
    }
    printf("C1 - [ ]\n");    
    return 0;
}

// It checks if the command is valid.
// In the future, if it is valid, run corresponding
// action for CN, where N = {1, 2, 3, 4}.
int validCommand(char **tokens) {
    printf("in valid command!\n");
    /*
    if (validC4(tokens))  return 1;*/
    if (validC3(tokens))  return 1;
    if (validC2(tokens))  return 1;
    if (validC1(tokens))  return 1;
    printf("INVALID COMMAND!\n");
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

char **parseCommand(char **args){
    char *token_1, *token_2, *token_3, *token_4;
    int i = 0;
    token_1 = calloc(BUFFER_SIZE, sizeof(char));
    assert(token_1 != NULL);
    while (args[i] != NULL && args[i][0] != '<' && args[i][0] != '>' && args[i][0] != '&') {
        printf("in loop with i = %d\n", i);
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

    free(token_1);
    free(token_2);
    free(token_3);
    free(token_4);

    return tokens;
    
}

void loop() {

    char *line = readLine();
    char **args = splitLine(line);
    char ** tokens = parseCommand(args);
    validCommand(tokens);
    
}

int main(int argc, char* argv[]) {
    loop();
}