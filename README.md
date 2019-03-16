## OS - Assignment 2
### Exercise 1 - Writing a shell
#### Set of special characters: {<, >, |, &, "}
### I/O redirection
**Redirection only happens between 1 output and 1 input.**
Redirection should be done in the launch phase of the program
as in this assignment it is required to have only one potential
file from which input can be received into the **first** argument
of the input_line. As such, per input line, only one I/O redirection
can be present.
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
- After every redirection arrow, at least one alpha numerical character must appear to designate the corresponding file.
- A command may not end with & unless it is the last command in the command_list.
- A command must exist in order for it to be fired.
- A file may not be used for redirecting both the input and the output.
After every check has passed the shell checks for I/O redirection files and saves the file names of them, if they are present in the input_line in order to set up file descriptors in the execution phase.
Then, the parser tokenizes the input_line into commands in two stages: first, the command is split separately by the pipe symbol ```|```. Afterwards, each resulting command is stored into an array of separated commands from which each command will fire.
Any command that has ```&``` will be fired immediately after encountering it, incrementing the number of children of the parent.
#### Execution phase
After a command is ready for execution the shell checks if it is an empty command: if it is then it returns 1 and waits for the next command,
otherwise the 

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

### TODO:
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
- [X] Implement background processes. Background processes are now possible within the shell by specifying the special character ```&```.
```bash
xeyes &
```
is a valid command and will start the process in background (**NEEDS FIXIN'**).
```bash
xeyes&
```
will also start a background process.
- [ ] Make sure to clean after defunct processes.
- [ ] When handling I/O, check if file exists before proceeding (leads to errors otherwise). Currently we detect files which are non-existent, but we do not handle them.
- [X] Execute I/O redirection commands.
- [X] Parse special characters
- [x] Execute regular commands
- [x] Execute regular commands with multiple flags
- [X] Execute composed commands
- [X] Execute commands with special characters
- [x] Implement cd/exit/help (extension)
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
