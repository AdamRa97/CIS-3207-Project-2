#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "helpers.h"

int pwd(void);
int cd(void);
int help(void);
int (*find_command(char *name))(void);

int main(){
    char input[1024];

    while (1){

        printf("> ");
        fgets(input, 1024, stdin);
        char * line = strdup(input);
        char ** array = parse(line," \n");

        if (array == NULL || array[0] == NULL)
            continue;

        int i = 0;
        while (array[i] != NULL){
            int (*command_func)(void) = find_command(array[i++]);

            if (command_func != NULL)
                command_func();
            
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

int cd(void){
    return 0;
}

int help(void){
    printf("pwd: prints out current working directory\n");
    printf("help: prints out built-in commands and their descriptions\n");
    printf("cd: changes directory\n");
    printf("exit: exits the shell\n");
    return 0;
}

// Find Command function that searches through the struct command array, and returns the pointer to it if found
int (*find_command(char *name))(void) {
    if (strcmp(name, "pwd") == 0)
        return &pwd;
    else if (strcmp(name, "cd") == 0)
        return &cd;
    else if (strcmp(name, "help") == 0)
        return &help;
    else if (strcmp(name, "exit") == 0)
        exit(0);
    else
        return NULL;
}
