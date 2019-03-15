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

#define BUFFER_SIZE 1024
#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"
#define INPUT 0
#define OUTPUT 1
#define PIPELINE 2
#define REGULAR 3
#define BACKGROUND 4

int MODE = REGULAR;

int hasSpecialCharacter(char **);
int isValidCommand(char*);
int isBuiltinCommand(char*);
int isFile(char*);
int isSpecChar(char);
int num_builtins(void);
int sh_cd(char **args);
int sh_help(char **args);
int sh_exit(char **args);
int launch(char**);
int execute(char**);
char **splitLine(char*);
char *readLine(void);
void shLoop(void);
void parseToken(char*);
void treatFileCommand(char*);
void treatSpecChar(char);
void treatValidCommand(char*);
void goTokens(char**);
/*
* Bread and butter. Should be refactored into functions for lizibility and modularity.
* Test cases need to be thought of.
*/
// while (args[i] != NULL) {
//     i++;
// }
// char **tokens = calloc(i + 2, sizeof(char*));
// i = 0;

// while (args[i] != NULL) {
//     if (args[i][0] == '<') {
//         if (args[i + 1] != NULL && args[i + 2] != NULL && args[i + 2][0] == '>') {
//             if (isFile(args[i + 1]) && isFile(args[i + 3])) {
//                 in = open(args[i + 1], O_RDONLY);
//                 out = open(args[i + 3], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
//                 dup2(in, 0);
//                 dup2(out, 1);
//                 close(in);
//                 close(out);
//             }
//             else {
//                 printf("--- FILES DO NOT EXIST ---\n");
//                 break;
//             }
//         }
//         else {
//             if (isFile(args[i + 1])) {
//                 in = open(args[i + 1], O_RDONLY);
//                 printf("in = %d\n", in);
//                 dup2(in, 0);
//                 close(in);
//                 tokens[i] = NULL;
//                 break;
//             }
//             else {
//                 printf("--- FILES DO NOT EXIST ---\n");
//                 break;
//             }
            
//         }
                 
//     }
//     if (args[i][0] == '>') {
//         out = open(args[i + 1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
//         dup2(out, 1);
//         close(out);
//         tokens[i] = NULL;
//         break;  
                        
//     }
//     tokens[i] = calloc(strlen(args[i]), sizeof(char));
//     strcpy(tokens[i], args[i]);
//     i++;
// }