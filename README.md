## OS - Assignment 2
### Exercise 1 - Writing a shell
#### Set of special characters: {<, >, |, &, "}
Need a parser for the input.
There are certain inputs which are possible.
#### Parser construction
Initial idea is to go through the input and create two buffers,
one for special characters and one for strings of characters.
Create two buffers of sizes that correspond to the number of special
characters / character strings and store the special characters and
strings into the buffer. An abstract structure may be created in order
to deal efficiently with the input.
The character '"' should parse everything as a string until the next '"'.
##### Chain of operations
1. Parse the input to check for valid expression.
2. Parse buffers to execute the expression.
3. Terminate processes that 

#### Possible inputs
1. After a "<" or a ">" you expect an input / output file. 
- This creates a pipe between the two files.
- \< redirects information into the lhs from the rhs.
- \> redirects information from the lhs into the rhs.
So, A > B should create a file descriptor that is used to communicate
between A and B. > will mean that A will write information to B.
This can be done by opening the file B and writing on B whatever output
A provides.
A \< B will mean that A will read information from B. This implies that
B should be opened and read until the eof. The information taken from
B should then be provided as input into A.
If no filename is provided: "Invalid syntax!".
2. [commandA] | [commandB] should create a new pipe between commandA
and commandB. 
If no command name is provided: 
3. 

#### Notes
- A potential function that deals with the creation of a file descriptor
between two processes might be useful since one does not know how many
file descriptors shall be needed in advance. As such, each operation
that needs a file descriptor shall create its own dynamically.
- dup() used for handling the output / input from a file to a file
