#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "helpers.h"

void pwd(void);
void cd(void);
void help(void);

typedef struct{
    const char *name;
    void (*func)(void);
} command;

int main(){
    char input[1024];
    command commands[] = {
        {"pwd", pwd},
        {"cd", cd},
        {"help", help}
    };

    printf("%s\n",commands[0].name);

    while (1){
        printf("> ");
        fgets(input, 1024, stdin);
        char * line = strdup(input);
        char ** array = parse(line," \n");

        if (array == NULL)
            exit(1);

        int i = 0;
        while (array[i] != NULL)
            printf("%s\n",array[i++]);

        
        free(array);
        free(line);
    }
    return 0;
}

void pwd(void){
    printf("hello");
}

void cd(void){
    printf("hello");
}

void help(void){
    printf("hello");
}