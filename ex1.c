#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "ex1.h"

int isFile(char* filename) {
	if (access(filename, F_OK ) != -1) {
    return 1;
  }
	return 0;
}

int isSpecChar(char c) {
		return ((c == '>') || (c == '<') || (c == '&') ||
		(c == '|') || (c == '"'));
}

int isBuiltinCommand(char *string) {
		if ((strcmp(string, "cd")) == 0 || (strcmp(string, "help") == 0) || (strcmp(string, "exit") == 0)) {
			return 1;
		}
		return 0;
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
	char shell_prompt[100];
    // Configure readline to auto-complete paths when the tab key is hit.
    rl_bind_key('\t', rl_complete);
	do {
		// printf("> ");
		// line = readLine();
		line = readline("> ");
		args = splitLine(line);
		goTokens(args);
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

void goTokens(char** tokens) {
	int i = 0;
	printf("Trying to parse tokens...\n");
	while (tokens[i] != NULL) {
		printf("token[%d] = %s\n", i, tokens[i]);
		parseToken(tokens[i]);
		i++;
	}
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
				exit(EXIT_FAILURE);
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

void treatSpecChar(char sc) {
	if (sc == '<')	printf("reading from rhs to lhs");
	if (sc == '>')	printf("writing to rhs from lsh");
	if (sc == '|')	printf("creating pipe with rhs argument");
	if (sc == '"')	printf("quotation");
	if (sc == '&')	printf("background process");
}

void treatValidCommand(char* command) {
	printf("doing some stuff with command\n");
}

void treatFileCommand(char *file) {
	printf("file shennanigans\n");
}

void parseToken(char* token) {
	printf("Parsing token...\n");
	printf("Token == %s\n", token);
	if (isSpecChar(token[0])) {
		treatSpecChar(token[0]);
	}
	if (isValidCommand(token)) {
		treatValidCommand(token);
	}
	if (isFile(token)) {
		treatFileCommand(token);
	}
}

void checkValidCommand() {
	char *command = readline("");
	printf("command = %s\n", command);
	if (isValidCommand(command)) {
		printf("Valid command!\n");
	}
	else {
		printf("Invalid command!\n");
	}
}

void checkExistFile() {
	char *filename = readline("");
	printf("filename = %s\n", filename);
	if (isFile(filename)) {
		printf("Existing file!\n");
	}
	else {
		printf("Non-existing file!\n");
	}
}

int main(int argc, char* argv[]) {
	shLoop();
	return EXIT_SUCCESS;
}
