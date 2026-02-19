# TechShell
A group project which works as a bash shell and can interpret some shell commands

Group Member Name(s):
Bryant Jee... That's it.

Responsibilities:
Bryant Jee - everything

Description:
This program is essentially a recreation of a bash shell with most of the commands implemented. 
It contains multiple functions which have several goals such as parsing input and executing commands.
These functions run on a continuous for loop that prompts the user for input through another function.
Some of the commands implemented in this program contain standard input and output through the '<' and '>' characters respectively.
Some additional ones are the cd, ls, cat, whereis, and ps commands.

to run this program, use git clone to create a local repo of the program on your computer, then open in 
the integrated terminal and compile through "gcc techshell.c -o <executable>", and then run with ./<executable>.

At the moment, the pipe character '|' does not work which is able to make an output of a command
become an input for another nor does it work with ';' to execute multiple commands at once. 
Additionnally, the appending token '>>' doesn't work with our program just being able to truncate output with '>'
