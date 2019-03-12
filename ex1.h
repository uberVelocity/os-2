#define BUFFER_SIZE 1024
#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"
#define INPUT 0
#define OUTPUT 1
#define PIPELINE 2
#define REGULAR 3

int hasSpecialCharacter(char **);
int isValidCommand(char*);
int isBuiltinCommand(char*);
int isFile(char*);
int isSpecChar(char);
int num_builtins(void);
int sh_cd(char **args);
int sh_help(char **args);
int sh_exit(char **args);
int launch(char**,int);
int execute(char**);
char **splitLine(char*);
char *readLine(void);
void shLoop(void);
void parseToken(char*);
void treatFileCommand(char*);
void treatSpecChar(char);
void treatValidCommand(char*);
void goTokens(char**);
