#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "helpers.h"

int pwd(void);
int cd(char *dir);
int help(void);
void execute_command(char** args);

#define MAX_COMMANDS 10 

int main(){
    char input[1024];

    while (1){
        printf("> ");
        fgets(input, 1024, stdin);
        char * line = strdup(input);
        char ** array = parse(line," \n");

        if (array == NULL || array[0] == NULL){
            free(line);
            free(array);
            continue;
        }

        int i = 0;
        char command[1024] = "";

         while (array[i] != NULL){
            if (strcmp(array[i], "exit") == 0) {
                free(line);
                free(array);
                exit(0);
            }
            else if (strcmp(array[i], "pwd") == 0)
                pwd();
            else if (strcmp(array[i], "cd") == 0){
                cd(array[i+1]);
                i++;
            }
            else if (strcmp(array[i], "help") == 0)
                help();
            else{
                strcat(command, array[i]);
                if (i < (MAX_COMMANDS - 1) && array[i+1] != NULL)
                    strcat(command, " ");
                else
                    execute_command(parse(command," "));
            }

            // Resetting command buffer
            command[0] = '\0';

            i++;
        }
        free(line);
        free(array);
    }
    return 0;
}

int pwd(void){
    char cwd[1024];
    if (getcwd(cwd,sizeof(cwd)) != NULL)
        printf("%s\n",cwd);
    else{
        perror("getcwd() error");
        return 1;
    }
    return 0;
}

int cd(char *dir){
    // If no directory was specified, change to the user's home directory
    if (dir == NULL){
        dir = getenv("HOME");
        if (dir == NULL){
            perror("cd: error: could not get home directory");
            return 1;
        }
    }

    // Change to the specified directory
    if (chdir(dir) != 0){
        perror("cd: error");
        return 1;
    }

    return 0;
}

int help(void){
    printf("pwd: prints out current working directory\n");
    printf("help: prints out built-in commands and their descriptions\n");
    printf("cd [path]: changes directory; if no path is entered then you go to the root directory\n");
    printf("exit: exits the shell\n");
    return 0;
}

void execute_command(char** args) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        // child process
        if (execv(args[0], args) == -1) {
            perror("execute_command");
            exit(EXIT_FAILURE);
        }
    } else if (pid < 0) {
        // fork error
        perror("execute_command");
    } else {
        // parent process
        waitpid(pid, &status, WUNTRACED);
    }
}