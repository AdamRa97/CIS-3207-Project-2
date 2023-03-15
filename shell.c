#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "helpers.h"

int pwd(void);
int cd(char *dir);
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

        // First checking if 'cd' was entered in; else search through if the other commands were entered
        if (strcmp(array[0], "cd") == 0) {
            // Call the 'cd' function with the specified directory
            cd(array[1]);
        }
        else{
            int i = 0;
            while (array[i] != NULL){
                int (*command_func)(void) = find_command(array[i++]);

                if (command_func != NULL)
                    command_func();
            }
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
    if (dir == NULL) {
        dir = getenv("HOME");
        if (dir == NULL) {
            perror("cd: error: could not get home directory");
            return 1;
        }
    }

    // Change to the specified directory
    if (chdir(dir) != 0) {
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

// Find Command function that searches through the struct command array, and returns the pointer to it if found
int (*find_command(char *name))(void) {
    if (strcmp(name, "pwd") == 0)
        return &pwd;
    else if (strcmp(name, "help") == 0)
        return &help;
    else if (strcmp(name, "exit") == 0)
        exit(0);
    else
        return NULL;
}
