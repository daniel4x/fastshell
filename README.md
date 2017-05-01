# fastshell

fastshell is an implementation of unix based os shell.</br>
I made this shell for self studying purpose using [this](https://brennan.io/2015/01/16/write-a-shell-in-c/) excellent tutorial. <br/>
fastshell is written in c, run and tested on OS X. 

## Compile and run
Compile fastshell with gcc: <br/>
`gcc fast.c -o fast`

## Additional information
- Currently implemented the following internal shell commands: cd, help and exit.
- External commands can also be executed, that thanks to fast_launch method. As part of learning how shell works i used fork() and exec() methodology in order to execute a different processes and wait them to finish. 

## fastshell routine
fastshell has a standard shell routine: 
- get user input: `fast_read_line();`
- parse user input: `fast_parse_line(line);`
- execute the command: `fast_execute(args);`

Feel free to create a pull request & code review :) 
