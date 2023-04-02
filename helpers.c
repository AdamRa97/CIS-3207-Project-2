#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

void safe_close(int fd);
char *resolve_command_path(const char *cmd);
int handle_builtin_commands(char** args);
void execute_piped_commands(char **args);
void execute_single_command(char **args, int in_fd, int out_fd, bool wait_for_completion);

#define PATH_MAX 4096

/*
  Function to break up a line (or any arbitrary string) into a memory allocated
  tokenized array of strings separated by the given delimiter or set of 
  delimiters (see strtok_r for more information about delim parameter). 
  
  Note: The array as well as the individual strings will be memory allocated,
  therefore they must be freed later. 
  
  Warning: LINE MUST BE MEMORY ALLOCATED!

  @param line : An arbitrary string
  @param args : An empty array to place the tokenized output
  @param delim : The delimiter or set of delimiters for separating 
    the line (see strtok_r for more information)
  
  @return argn : The length of the array
*/

void safe_close(int fd){
    if (close(fd) == -1){
        fprintf(stderr, "Error: safe_close failed for fd %d\n", fd);
        perror("safe_close");
    }
}

char **parse(char *line, char *delim){
    char **array = malloc(sizeof(char *));
    *array = NULL;
    int n = 0;

    char *buf = strtok(line, delim);

    if (buf == NULL){
        free(array);
        array = NULL;
        return array;
    }

    while (buf != NULL){
        char **temp = realloc(array, (n + 2) * sizeof(char *));

        if (temp == NULL){
            free(array);
            array = NULL;
            return array;
        }

        array = temp;
        temp[n] = strdup(buf);  // Create a new string for the token
        temp[n + 1] = NULL;
        n++;

        buf = strtok(NULL, delim);
    }

    return array;
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
 * Returns an input stream to be used depending on if the shell is launched in interactive / batch mode
 * Best used with getline https://man7.org/linux/man-pages/man3/getline.3.html
 */
FILE *getInput(int argc, char* argv[]){
    //if the argument count is 2, attempt to open the file passed to the shell
    FILE *mainFileStream = NULL;
    if(argc == 2){
        mainFileStream = fopen(argv[1], "r");
        if(mainFileStream == NULL){
            printf("Error opening batch file\n");
            exit(1);
        }
    }
    //set the file stream to standard input otherwise
    else if(argc ==1)
        mainFileStream = stdin;
    else{
        printf("Too many arguments\n");
        exit(1);
    }
    return mainFileStream;
}

void execute_command(char** args){
    if (args[0] == NULL)
        return;

    if (handle_builtin_commands(args))
        return;

    int num_pipes = 0;
    for (int i = 0; args[i] != NULL; ++i){
        if (strcmp(args[i], "|") == 0)
            num_pipes++;
    }

    if (num_pipes > 0)
        execute_piped_commands(args);
    else{
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


char *resolve_command_path(const char *cmd){
    char *env_path = getenv("PATH");

    if (env_path == NULL){
        fprintf(stderr, "resolve_command_path: error: PATH not set\n");
        return NULL;
    }

    // Duplicate the PATH environment variable
    char *path = strdup(env_path);
    char *dir = strtok(path, ":");

    while (dir != NULL){
        char cmd_path[1024];
        snprintf(cmd_path, sizeof(cmd_path), "%s/%s", dir, cmd);

        if (access(cmd_path, X_OK) == 0){
            // Command found in this directory
            char *resolved_path = strdup(cmd_path);
            free(path);
            return resolved_path;
        }

        dir = strtok(NULL, ":");
    }

    free(path);
    return NULL;
}

int handle_builtin_commands(char **args){
    if (strcmp(args[0], "exit") == 0)
        exit(EXIT_SUCCESS);
    else if (strcmp(args[0], "help") == 0){
        printf("Type the name of a command followed by arguments and press enter.\n");
        printf("Built-in commands:\n");
        printf("  exit - exit the shell\n");
        printf("  help - display this help message\n");
        printf("  cd - change directory\n");
        printf("  pwd - print working directory\n");
        return 1;
    }
    else if (strcmp(args[0], "cd") == 0){
        if (args[1] == NULL)
            fprintf(stderr, "cd: expected argument\n");
        else{
            if (chdir(args[1]) != 0)
                perror("cd");
        }
        return 1;
    } 
    else if (strcmp(args[0], "pwd") == 0){
        char cwd[PATH_MAX];

        if (getcwd(cwd, sizeof(cwd)) != NULL)
            printf("%s\n", cwd);
        else
            perror("pwd");
        
        return 1;
    }
    else if (strcmp(args[0], "echo") == 0){
        if (args[1] != NULL && args[1][0] == '$'){
            // Remove the $ sign from the variable name
            char *env_var = args[1] + 1;
            char *env_value = getenv(env_var);
            if (env_value)
                printf("%s\n", env_value);
            else
                printf("\n");
            return 1;
        }
    }
    else if (strcmp(args[0], "wait") == 0){
        int status;
        pid_t pid;

        while ((pid = wait(&status)) > 0);

        return 1;
    }
    return 0;
}

void execute_piped_commands(char **args){
    int pipe_count = 0;
    for (int i = 0; args[i] != NULL; ++i){
        if (strcmp(args[i], "|") == 0)
            pipe_count++;
    }

    int num_cmds = pipe_count + 1;
    char ***cmds = malloc(num_cmds * sizeof(char **));

    int cmd_index = 0;
    cmds[cmd_index++] = args;
    for (int i = 0; args[i] != NULL; ++i){
        if (strcmp(args[i], "|") == 0){
            args[i] = NULL;
            cmds[cmd_index++] = args + i + 1;
        }
    }

    int *pipes = malloc(2 * pipe_count * sizeof(int));
    for (int i = 0; i < pipe_count; ++i){
        if (pipe(pipes + 2 * i) == -1){
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_cmds; ++i){
        int in_fd = (i == 0) ? -1 : pipes[2 * (i - 1)];
        int out_fd = (i == num_cmds - 1) ? -1 : pipes[2 * i + 1];

        execute_single_command(cmds[i], in_fd, out_fd, (i == num_cmds - 1));

        // Close file descriptors after executing the command
        if (in_fd != -1)
            safe_close(in_fd);
        if (out_fd != -1)
            safe_close(out_fd);

        // Wait for the child process to complete
        int status;
        wait(&status);
    }

    free(cmds);
    free(pipes);
}

void execute_single_command(char **args, int in_fd, int out_fd, bool wait_for_completion){
    if (args[0] == NULL)
        return;

    if (handle_builtin_commands(args))
        return;

    // Check if the command should be run in the background
    bool run_in_background = false;
    int num_args = 0;
    while (args[num_args] != NULL){
        num_args++;
    }
    if (num_args > 0 && strcmp(args[num_args-1], "&") == 0){
        run_in_background = true;
        args[num_args-1] = NULL;
    }

    char *cmd_path = resolve_command_path(args[0]);
    if (cmd_path == NULL){
        fprintf(stderr, "execute_single_command: %s: command not found\n", args[0]);
        return;
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
}
