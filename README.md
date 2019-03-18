# OS - Assignment 2
## Authors: Mihai Popescu & Andrei Scurtu
## Exercise 1 - Writing a shell

### Problem description
The scope of the assignment was to build a pseudo-linux shell which supports the following:
- Start a program which can be found in the user's search ($PATH)
- I/O redirection
- Background processes
- Pipes
- String parsing

#### Set of special characters: {<, >, |, &, "}

### Start a program
The input line is parsed such that a command is tokenized into its arguments. In order to simulate a shell environment, the shell must be able to receive multiple inputs one after another, and as such the command is executed by a children of the shell. This is done by envoking the ```fork()``` function followed by ```execvp()```. 

### I/O redirection
**Redirection only happens between 1 output file and 1 input file.**
Redirection is handled differently in this assignment than the way it is implemented in Linux.
There are **at most** two redirection files: an input file and an output file. They lie anywhere in the input_line given by the user. The input redirection should be connected to the first command of the input_line, whilst the output redirection should be connected to the last command of the input_line.
If a redirection of the form occurs:
```bash
a.out | b.out | c.out < in
```
the input of a.out is ```in```.
If a redirection of the form occurs:
```bash
a.out | b.out < in | c.out > out
```
the input of ```a.out``` will be ```in``` and the final output will
be in ```out```. 
The logic is as follows:
1. The input of ```a.out``` will be ```in```.
2. ```a.out``` executes.
3. The output of ```a.out``` will be given to ```b.out``` as input.
4. ```b.out``` executes.
5. The output of ```b.out``` is given to ```c.out``` as input.
6. ```c.out``` executes.
7. The output of ```c.out``` will be written in the ```out``` file.

### Background processes
Background processes are simply regular processes that the shell does not wait for them to exit.
They were supported at one point during the shell by not waiting for the process to exit, however this proved
to cause orphan processes. One implementation that might deal with defunct processes is to store the pid of each process in the parent through an array. Knowing how many processes the parent has created, it would know for how many processes it should wait for. Unfortunately, we have not implemented this in the final version of the shell.

### Pipes
Pipes are implemented using file descriptors.

### String parsing
The shell interprets quotes "" as one meaningful chunk. As such, for inputs such as
```bash
grep "test < a" test
```
the shell would transform it into three tokens: {grep, test < a, test}. After communication with the T.A.s it seemed that only the outermost quotes should be used in order to group tokens and that test cases in which an odd number of quotes would be presented would not be used. The shell, for as much as we know, implements the parsing of strings correspondingly even though it does not passes Themis in this scenario.

### Shell construction
The shell has three phases.
1. The shell waits for an input_line from the user.
2. The shell checks if the input_line has a valid syntax with the grammar defined in the assignment.
3. The shell executes the input_line.

#### Input phase
The shell implements a dynamically allocated buffer in which it stores the input received from the user.

#### Validation of the input_line
The shell must check for the following scenarios in order for the input_line to pass to the execution phase:
- The input_line cannot end with a 'special character' apart from &.
- After every redirection arrow, at least one alpha numerical character or dot must appear to designate the corresponding file.
- A command must exist in order for it to be fired.
- A file may not be used for redirecting both the input and the output.

After every check has passed, the shell checks for I/O redirection files and saves the file names of them, if they are present in the input_line in order to set up file descriptors in the execution phase.
Then, the parser tokenizes the input_line into commands in two stages: first, the command is split separately by the pipe symbol ```|```. Afterwards, each resulting command is stored into an array of separated commands from which each command will fire.
Any command that has ```&``` will be fired immediately after encountering it, incrementing the number of children of the parent (removed).
#### Execution phase
After a command is ready for execution the shell checks if it is an empty command: if it is then it returns 1 and waits for the next command. Otherwise, it fires the command.

### Possible inputs
1. After a "<" or a ">" you expect an input / output file. 
- \< redirects information into the lhs from the rhs.
- \> redirects information from the lhs into the rhs.
So, A > B should create a file descriptor that is used to communicate
between A and B. > will mean that A will write information to B.
This can be done by opening the file B and writing on B whatever output
A provides.
A \< B will mean that A will read information from B. This implies that
B should be opened and read until the eof. The information taken from
B should then be provided as input into A.
2. [commandA] | [commandB] should create a new pipe between commandA
and commandB.
Pipes and input/output redirection [example #1](http://www.cs.loyola.edu/~jglenn/702/S2005/Examples/dup2.html), [example #2](https://www.unix.com/programming/122360-c-piping-redirect-operator.html), [example #3](https://www.geeksforgeeks.org/making-linux-shell-c/)

### Explanations
- [X] Add special character '"' functionality: Functionality has been added and now commands including quotes are parsed appropriately.
As an example, consider the command:
```bash
echo "some string <> with 'special' characters"
```
The command above will be tokenized into two tokens: ```echo``` and ```some string <> with 'special' characters``` It will print on the screen ```some string <> with 'special' characters```.
The trick is to take everything between the quotes as one argument of the command.
Example: <br/>
```bash
git commit -m "this is a message"
```
This command is supported by the shell. The git command will require the quotes to remain in the execution parameters of the command in order for git to parse the contents within the quotes as the body of the message.
```bash
cat "some file that's on the computer"
```
In this scenario, bash uses the quotes to logically group and evaluate the values of each variable between the quotes. Functionality-wise, it transforms the command into
```bash
cat some\ file\ that's\ on\ the\ computer
```
This is also supported by the shell as it takes as parameters to the command (```args[0] = cat, args[1] = some file that's on the computer```. Whatever ```cat``` chooses to do with the command (transform it into ```cat some\ file\ that's\ on\ the\ computer```) is up to it.
- [X] Implement background processes. Background processes were possible within the shell by specifying the special character ```&```.
```bash
xeyes &
```
was a valid command and it would start the process in background
```bash
xeyes&
```
will also start a background process.
### Extensions
As far as extensions go, we have implemented ```help``` and ```cd```.
- [X] Retrieve history of commands using up-arrow key using Readline library.
- [x] Implement cd/help (extension)
- [x] Auto-complete path names with tab (extension - GNU Readline)

### Remaining Extensions:
- tab twice to show possibilities
- provide [emacs shortcuts](https://en.wikipedia.org/wiki/GNU_Readline)
- change environmental variables using export (can be used to implement [colors](https://www.cyberciti.biz/faq/bash-shell-change-the-color-of-my-shell-prompt-under-linux-or-unix/))
### Notes
- A potential function that deals with the creation of a file descriptor
between two processes might be useful since one does not know how many
file descriptors shall be needed in advance. As such, each operation
that needs a file descriptor shall create its own dynamically.
- dup2() used for handling the output / input from a file to a file
- redirection of input output happens before execute and after fork.
- useful tutorial https://brennan.io/2015/01/16/write-a-shell-in-c/
