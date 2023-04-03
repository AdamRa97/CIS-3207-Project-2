int find_special (char*args[], char * special);
FILE *getInput(int argc, char* argv[]);
void execute_command(char** args);
void sigchld_handler(int sig);