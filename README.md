# Project-2-S23
## Project 2: Shell
In this homework, you’ll be building a shell, similar to the tsch shell you use on your CIS Linux2 system. When you open a terminal window on your computer, you are running a shell program, which is tsch on your system. The purpose of a shell is to provide an interface for users to access an operating system’s services, which include file and process management. sh (Bourne shell) is the original Unix shell, and there are many different flavors of shells available. Some other examples include ksh (Korn shell), bash (GNU Bourne again shell), and zsh (Z shell). Shells can be interactive or non-interactive. 

For instance, you are using tsch non-interactively when you run a tsch script. tsch is interactive when invoked without arguments, or when the -i flag is explicitly provided. The operating system kernel provides well-documented interfaces for building shells. By building your own, you’ll become more familiar with these interfaces, and you’ll probably learn more about other shells as well. 

# Adam Ra Documentation

## Pseudocode
Early Progress Pseudocode
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

help func{
    print out built-in commands and how to use shell;
}

pwd func{
    use getcwd();
}

cd func{
    use getenv() and chdir();
}

## Program Details
This program is a shell that replicates some of the same behaviors shown with the bash shell. It is able to run piped commands, have redirection, uses it's own PATH resolution, and run background processes

It has 5 built in commands (cd, help, exit, pwd, wait)
"cd" can be used just as the bash shell
"help" will print out how to use the built in commands
"exit" will do exactly what sounds
"pwd" will print out the current working directory
"wait" does exactly what it says

## Deliverables
### Weekly 1 Deliverables
* pseudocode document
* create core shell for built-ins
* help & exit built-in commands
* pwd built-in
* cd built-in

Pseudocode document was fairly small and very simple, might even say incomplete. Didn't know at all how I would want to start with the project at first. Actually went through several different implementation approaches
that forced me to refactor my code (huge waste of time). Pseudocode Document is very important but what's more important is a design doc implementation with different approaches with pro's and cons

Creating the core shell for built-ins was pretty simple at first but eventually had to change how I implemented it later on (still pretty much the same)

The Help & Exit built in commands were fairly simple as well, I decided to go for the hard code route since I felt that reading into a file and outputting the contents felt unnecessary
for a small amount of built in commands. If the help command were to print out the full context of the shell then I would had read into it and output the content instead.
The exit command was fairly simple as it is essentially just calling the same name.

pwd was something that I had done previously or at least something similar with cwd with project 0 (ls program) so that was fairly easy to implement

cd was also something easy to implement cause of previous research done for project 1 (proc file program) I saw something about chdir. (I think I actually used chdir for proc file system)

========================================================================================================================================================================================================
### Weekly 2 Deliverables
* proper use of information provided by the PATH environment variable
* implementation of redirection
* correct implementation of redirection of stdout
* correct implementation of redirection of stdin
* commandline processing of redirection of stdin and stdout
* implementation of pipe [|]
* implementation of indefinite pipe commands on one commandline

Having a proper implementation of my own PATH resolution was really annoying to figure out since we were restricted from using execvp. I had to duplicate the old path so that it wouldn't get overriden
with future / new paths

The redirection was also fairly annoying to figure out and took me quite some time but not the worst. Had to start utilizing file descriptors a bit more and had to be more familiar with how they worked.

The stdout, stdin, and parsing of the command line was actually not too bad, seemed a little straight forward for me. Had to research some things to clarify about stuff I wasn't too sure about (mainly parsing)

Implementations of pipes wasn't too bad until I had to figure out how to handle indefinite pipe commands on one command line. (huge headache)
Took me at least around a full day or two (20+ hrs) of just adding new functions and using pre-existing functions to figure out what would work. I was stuck on having only one pipe command working for a while

========================================================================================================================================================================================================

### Weekly 3 Deliverables / Full Project
* implementation of running a program in the background
* implementation of a wait built-in command

Having to parse the & wasn't too bad and utilized the same techniques I used to see if pipes or redirection was in the command line. Got it to work fairly quickly but the hardest part
for sure was having it be compatible with my other functions that handled redirection and pipes. I actually didn't test it with redirection but mainly pipes e.g "ls | grep shell | sleep 5 &" and I
would use "ps" to make sure the process was actually running in the background. I came across some defunct processes after they ended and realized I forgot to include a way to reap them.
very simple process with signal handling so that was pleasantly surprising to me after being stuck on pipes and background processes for a while.

The wait built in command confused me a little bit in terms of why it was on weekly deliverables 3 rather than included with the first 4 (help, cd, pwd, and exit)
This was fairly simple to build as well with the rest of the built in commands.

========================================================================================================================================================================================================

## Testing Methods
I would heavily use the "ps" command whenever testing for background processes to make sure if they were running and if they stayed or were gone from the ps list. (defunct or not)

I would also heavily use printf statements to figure out which PID's were running for my child processes / parent proceses to figure out the forking as well as the execvs

When handling with the file descriptors for the piped commands and redirection, I was stuck on it for the longest time until I decided to use printf's and see what exactly was going on
and when.

Tried implementing different .c files rather than bulking up helpers.c file since it was starting to get uncomfortably too big for my taste (150+ lines) in terms of helper functions.

Supplied "parse" code was faulty at first but was later fixed by a TA. The new parse code was fine for a while but later on it just wasn't good enough to be compatible with my current code.
So I decided to modify it to match my needs but because of how I modified it I needed to move it to the shell.c

Huge thanks to google and stackoverflow questions whenever I was stumped (basically throughout the whole project) to help me figure out what exactly was wrong with my code or a better way to approach it
(remembered about ternary operators and how to utilize it efficiently)

The following is what I would I enter in my command line to test if my code works (I also use makefile to compile everything fairly easily)

dir
pwd
ls | wc 
ls > file
echo "file"
wc < file

echo $PATH

ls | grep shell
ls | grep shell | wc 
ls | grep shell | sleep 5 &
sleep 5 & 
sleep 2

exit