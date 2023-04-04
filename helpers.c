#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

void safe_close(int fd);
char *resolve_command_path(const char *cmd);
int handle_builtin_commands(char** args);
void execute_piped_commands(char **args);
pid_t execute_single_command(char **args, int in_fd, int out_fd, bool wait_for_completion);

#define PATH_MAX 4096

// to properly / safely close out file descriptors
void safe_close(int fd){
    if (close(fd) == -1){
        fprintf(stderr, "Error: safe_close failed for fd %d\n", fd);
        perror("safe_close");
    }
}

/*Returns index of first instance of char * special*/
int find_special (char*args[], char * special){
	int i = 0;
	while(args[i]!=NULL){
		if(strcmp(args[i],special)==0)
			return i;
		i++;
	}
	return -1;
}

/* 
    RUBRIC RELATED POINTS
    |Implementation on both pipes and redirection|
    To execute through commands and calls in 'execute_single_command' or 'execute_piped_commands'
    depending whether there are '|'.
*/
void execute_command(char** args){
    // Checks if there is anything to even execute
    if (args[0] == NULL)
        return;
    
    // Checks if any of the built-in commands were typed in
    if (handle_builtin_commands(args))
        return;

    // Checking if pipes were given in command line
    int num_pipes = 0;
    for (int i = 0; args[i] != NULL; ++i){
        if (strcmp(args[i], "|") == 0)
            num_pipes++;
    }

    // Executing piped commands if there are pipes
    if (num_pipes > 0)
        execute_piped_commands(args);
    else{
        // Creating file descriptor variables for future when actually inputting or outputting
        int in_fd = -1;
        int out_fd = -1;

        // Find input/output redirection operators
        int input_redirect_index = find_special(args, "<");
        int output_redirect_index = find_special(args, ">");

        // Handle input redirection
        if (input_redirect_index != -1){
            in_fd = open(args[input_redirect_index + 1], O_RDONLY);
            if (in_fd == -1){
                perror("execute_command: open input file");
                exit(EXIT_FAILURE);
            }
            args[input_redirect_index] = NULL;
        }

        // Save the output file name before setting the element to NULL
        char *output_file = NULL;
        if (output_redirect_index != -1){
            output_file = args[output_redirect_index + 1];
            args[output_redirect_index] = NULL;
        }

        // Handle output redirection
        if (output_file != NULL){
            out_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (out_fd == -1){
                perror("execute_command: open output file");
                exit(EXIT_FAILURE);
            }
        }

        execute_single_command(args, in_fd, out_fd, true);

        // Close file descriptors after executing the command
        if (in_fd != -1)
            safe_close(in_fd);

        if (out_fd != -1)
            safe_close(out_fd);
    }
}

/*
    RUBRIC RELATED POINTS
    |Program execution from the shell with full program path|
    This function implements my own PATH since we aren't allowed to use execvp
    and need to use our own PATH resolution
*/
char *resolve_command_path(const char *cmd){
    char *env_path = getenv("PATH");

    // Error checking
    if (env_path == NULL){
        fprintf(stderr, "resolve_command_path: error: PATH not set\n");
        return NULL;
    }

    // Duplicate the PATH environment variable to store old PATH
    char *path = strdup(env_path);
    // Tokenizing each directory
    char *dir = strtok(path, ":");

    // Concatenating the strings together to create the full PATH
    while (dir != NULL){
        char cmd_path[1024];
        snprintf(cmd_path, sizeof(cmd_path), "%s/%s", dir, cmd);

        if (access(cmd_path, X_OK) == 0){
            // Command found in this directory
            char *resolved_path = strdup(cmd_path);
            free(path);
            return resolved_path;
        }
        // If command not found, move to the next directory token
        dir = strtok(NULL, ":");
    }
    free(path);
    return NULL;
}

/*
    RUBRIC RELATED POINTS
    |Built in Commands for help, exit, pwd, cd, and wait|
*/
int handle_builtin_commands(char **args){
    // Exit built-in command
    if (strcmp(args[0], "exit") == 0)
        exit(EXIT_SUCCESS);
    /*
        Help built in command, decided to just hard code in the msgs
        Reason for this is because reading into a file with the msg contents
        is unnecessary with few built-in commands
    */
    else if (strcmp(args[0], "help") == 0){
        printf("Type the name of a command followed by arguments and press enter.\n");
        printf("Built-in commands:\n");
        printf("  exit - exit the shell\n");
        printf("  help - display this help message\n");
        printf("  cd - change directory\n");
        printf("  pwd - print working directory\n");
        return 1;
    }
    // CD built in command 
    else if (strcmp(args[0], "cd") == 0){
        // Error checks to make sure given argument is able to CD to
        if (args[1] == NULL)
            fprintf(stderr, "cd: expected argument\n");
        else{
            // Checks if the return is a success or not
            if (chdir(args[1]) != 0)
                perror("cd");
        }
        return 1;
    } 
    // PWD built in command
    else if (strcmp(args[0], "pwd") == 0){
        char cwd[PATH_MAX];

        // Error checks while also printing out the contents if it works
        if (getcwd(cwd, sizeof(cwd)) != NULL)
            printf("%s\n", cwd);
        else
            perror("pwd");
        return 1;
    }
    // Echo built in command
    else if (strcmp(args[0], "echo") == 0){
        // Remove the $ sign from the variable name
        if (args[1] != NULL && args[1][0] == '$'){
            char *env_var = args[1] + 1;
            char *env_value = getenv(env_var);
            // Printing actual $ values e.g 'echo $PATH'
            if (env_value)
                printf("%s\n", env_value);
            else
                printf("\n");
            return 1;
        }
    }
    // Wait built-in command
    else if (strcmp(args[0], "wait") == 0){
        int status;
        pid_t pid;

        while ((pid = wait(&status)) > 0);

        return 1;
    }
    return 0;
}

void execute_piped_commands(char **args){
    // Calculating how many pipes are in the command line
    int pipe_count = 0;
    for (int i = 0; args[i] != NULL; ++i){
        if (strcmp(args[i], "|") == 0)
            pipe_count++;
    }

    // Calculating how many commands there are
    int num_cmds = pipe_count + 1;
    char ***cmds = malloc(num_cmds * sizeof(char **));

    // Splitting the arguments
    int cmd_index = 0;
    cmds[cmd_index++] = args;
    for (int i = 0; args[i] != NULL; ++i){
        if (strcmp(args[i], "|") == 0){
            args[i] = NULL;
            cmds[cmd_index++] = args + i + 1;
        }
    }

    // Allocating memory which stores the fd for the pipes and creates pipes
    int *pipes = malloc(2 * pipe_count * sizeof(int));
    for (int i = 0; i < pipe_count; ++i){
        if (pipe(pipes + 2 * i) == -1){
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Check if the command should be run in the background
    bool run_in_background = false;
    int num_args = 0;
    while (args[num_args] != NULL)
        num_args++;
    if (num_args > 0 && strcmp(args[num_args - 1], "&") == 0){
        run_in_background = true;
        args[num_args - 1] = NULL;
    }

    /*
        loop iterates through the cmds and for each of em, sets up input and output fd depending on the position
        also utilizes ternary operator (?)
    */
    for (int i = 0; i < num_cmds; ++i){
        int in_fd = (i == 0) ? -1 : pipes[2 * (i - 1)];
        int out_fd = (i == num_cmds - 1) ? -1 : pipes[2 * i + 1];

        bool wait_for_completion = ((i == num_cmds - 1) && !run_in_background);
        execute_single_command(cmds[i], in_fd, out_fd, wait_for_completion);

        // Close file descriptors after executing the command
        if (in_fd != -1)
            safe_close(in_fd);
        if (out_fd != -1)
            safe_close(out_fd);
    }
    free(cmds);
    free(pipes);
}

pid_t execute_single_command(char **args, int in_fd, int out_fd, bool wait_for_completion){
    if (args[0] == NULL)
        return -1;

    if (handle_builtin_commands(args))
        return -1;

    // Check if the command should be run in the background
    bool run_in_background = false;
    int num_args = 0;
    while (args[num_args] != NULL)
        num_args++;
    if (num_args > 0 && strcmp(args[num_args-1], "&") == 0){
        run_in_background = true;
        args[num_args-1] = NULL;
    }

    char *cmd_path = resolve_command_path(args[0]);
    if (cmd_path == NULL){
        fprintf(stderr, "execute_single_command: %s: command not found\n", args[0]);
        return -1;
    }

    pid_t pid = fork();

    if (pid == 0){
        // Child process
        if (in_fd != -1){
            if (dup2(in_fd, STDIN_FILENO) == -1){
                perror("execute_single_command: dup2 input");
                exit(EXIT_FAILURE);
            }
            safe_close(in_fd);
        }

        if (out_fd != -1){
            if (dup2(out_fd, STDOUT_FILENO) == -1){
                perror("execute_single_command: dup2 output");
                exit(EXIT_FAILURE);
            }
            safe_close(out_fd);
        }

        if (execv(cmd_path, args) == -1){
            perror("execute_single_command");
            exit(EXIT_FAILURE);
        }
    } 
    else if (pid < 0){
        // Fork error
        perror("execute_single_command");
        exit(EXIT_FAILURE);
    } 
    else{
        // Parent process
        if (!run_in_background && wait_for_completion){
            int status;
            waitpid(pid, &status, 0);
        }
    }
    // Free the allocated memory for cmd_path
    free(cmd_path);
    return pid;
}

// function to handle signals which will be used later in shell.c
void sigchld_handler(int sig){
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0);
}