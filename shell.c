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
int parse_line(char *line, char **args);

#define MAX_LINE_LENGTH 1024

int main(){
    char line[MAX_LINE_LENGTH];
    char *args[MAX_LINE_LENGTH / 2 + 1];

    while (1){
        printf("> ");
        if (fgets(line, MAX_LINE_LENGTH, stdin) == NULL){
            printf("\n");
            exit(EXIT_SUCCESS);
        }

        int num_args = parse_line(line, args);

        // Remove newline character from the last argument if present
        int last_arg_index = 0;

        while (args[last_arg_index] != NULL)
            last_arg_index++;

        if (last_arg_index > 0){
            char *last_arg = args[last_arg_index - 1];
            int last_arg_length = strlen(last_arg);

            if (last_arg[last_arg_length - 1] == '\n')
                last_arg[last_arg_length - 1] = '\0';
        }

        if (num_args == -1){
            fprintf(stderr, "Error: too many arguments\n");
            continue;
        }

        execute_command(args);
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
            fprintf(stderr,"cd: error: could not get home directory");
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

int parse_line(char *line, char **args){
    int num_args = 0;
    char *token = strtok(line, " \t");

    while (token != NULL){
        args[num_args] = token;
        num_args++;

        if (num_args >= MAX_LINE_LENGTH / 2)
            return -1;

        token = strtok(NULL, " \t");
    }

    args[num_args] = NULL;
    return num_args;
}