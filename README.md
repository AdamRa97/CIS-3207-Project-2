# Project-2-S23
## Project 2: Shell
In this homework, you’ll be building a shell, similar to the tsch shell you use on your CIS Linux2 system. When you open a terminal window on your computer, you are running a shell program, which is tsch on your system. The purpose of a shell is to provide an interface for users to access an operating system’s services, which include file and process management. sh (Bourne shell) is the original Unix shell, and there are many different flavors of shells available. Some other examples include ksh (Korn shell), bash (GNU Bourne again shell), and zsh (Z shell). Shells can be interactive or non-interactive. 

For instance, you are using tsch non-interactively when you run a tsch script. tsch is interactive when invoked without arguments, or when the -i flag is explicitly provided. The operating system kernel provides well-documented interfaces for building shells. By building your own, you’ll become more familiar with these interfaces, and you’ll probably learn more about other shells as well. 

# Adam Ra Documentation

## Pseudocode
In Progress Pseudocode (Updates over time while continuing to work on the project)
----------------------------------------------------------------------------------
import all stuff here;

int main(void){
    declare any variables needed here;

    while(1){
        console output;
        scan output;
        allocate memory and utilize parse from helpers.c;

        check for errors within the output;

        if no errors{
            check for any built-in commands;

            if no built-in commands{
                fork() and execv();
            }

            if 'exit' was entered, just use exit();
        }
    }
    free any memory;
}

<!-- Temporary Solution, will find a way to read into a file and print that output -->
help func{
    print out built-in commands and how to use shell;
}

pwd func{
    use getcwd();
}

cd func{
    use getenv() and chdir();
}