#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "helpers.h"

int parse_line(char *line, char **args);

// need to define this variable since parse_line utilizes it
#define MAX_LINE_LENGTH 1024

int main(){
    // Creating arrays to store arguments parsed from command line
    char line[MAX_LINE_LENGTH];
    char *args[MAX_LINE_LENGTH / 2 + 1];

    // Register the signal handler for SIGCHLD
    signal(SIGCHLD, sigchld_handler);

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

        // error checks if there are somehow less than 0 arguments parsed
        if (num_args == -1){
            fprintf(stderr, "Error: too little arguments\n");
            continue;
        }
        execute_command(args);
    }

    return 0;
}

/*
    RUBRIC RELATED POINTS
    |Proper use of supplied parse code|
    I modified the parse code to use my own that is compatible with the helper.c functions.
*/
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