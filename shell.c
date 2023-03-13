#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "helpers.h"
#include <stdbool.h>

int pwd(void);
int cd(void);
int help(void);
int (*find_command(char *name))(void);

// A struct designed for commands, has name property and a pointer to the func
typedef struct{
    const char *name;
    int (*func)(void);
} command;

// Array of built-in commands
command commands[] = {
    {"pwd", pwd},
    {"cd", cd},
    {"help", help}
};

int main(){
    char input[1024];
    bool status = true;

    while (status){
        printf("> ");
        fgets(input, 1024, stdin);
        char * line = strdup(input);
        char ** array = parse(line," \n");

        if (array == NULL)
            exit(1);

        int i = 0;
        while (array[i] != NULL){
            printf("%s\n",array[i]);
            int (*command_func)(void) = find_command(array[i]);

            if (command_func != NULL)
                status = command_func();
            // else
            //     printf("Command not found: %s\n", array[i]);

            i++;
        }

        free(line);
        free(array);
    }
    return 0;
}

int pwd(void){
    printf("hello\n");
    return 0;
}

int cd(void){
    printf("hello\n");
    return 0;
}

int help(void){
    printf("hello\n");
    return 0;
}


// Find Command function that searches through the struct command array, and returns the pointer to it if found
int (*find_command(char *name))(void) {
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(name, commands[i].name) == 0) {
            return commands[i].func;
        }
    }
    return NULL;
}
