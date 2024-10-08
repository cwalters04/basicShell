#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARG_SIZE 100

void shellLoop();
char *readLine();
char **parseLine(char *line);
int executeCommand(char **args);

int main() {
    // Start shell loop
    shellLoop();
    return 0;
}

// Command should allow user to get the current directory
char* cdCommand(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "myshell: expected argument to \"cd\"\n");
        return NULL;
    }
    
    if (chdir(args[1]) != 0) {
        perror("myshell");
        return NULL;
    }

    char *cwd = malloc(MAX_INPUT_SIZE * sizeof(char));
    if (cwd == NULL) {
        perror("myshell");
        return NULL;
    }

    if (getcwd(cwd, MAX_INPUT_SIZE) == NULL) {
        perror("myshell");
        free(cwd); // Free allocated memory on failure
        return NULL;
    }

    return cwd;
}

// Run the shell until 'exit'
void shellLoop(){
    char *line;
    char **args;
    int status;
    char *currentDir;
    do{
        // Get current directory before printing the prompt
        currentDir = getcwd(NULL, 0);
        if(currentDir != NULL){
            // Print shell prompt with current directory
            printf("myshell> %s> ", currentDir);
            free(currentDir); // Free memory allocated to by getcwd
        } else {
            printf("myshell> ");
        }
        
        // Read input from the user
        line = readLine();
        
        // Parse inputs into arguments
        args = parseLine(line);

        // Execute the command
        status = executeCommand(args);

        // Free allocated memory
        free(line);
        free(args);
    } while(status);
}

// Read the line given by the user
char *readLine(){
    char *line = NULL;
    size_t bufsize = 0; // Buffer size automatically allocated by getline

    if(getline(&line, &bufsize, stdin) == -1){
        if (feof(stdin))
        {
            exit(EXIT_SUCCESS); // An EOF was received
        } else
        {
            perror("readline");
            exit(EXIT_FAILURE); // Failed to recieve EOF
        }
        }
    return line;
}
// Parse the input so that the computer can read & execute input
char **parseLine(char *line) {
    int bufsize = MAX_ARG_SIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens)
    {
        fprintf(stderr, "myshell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, " \t\r\n\a");
    while (token != NULL){
        tokens[position] = token;
        position++;

        if (position >= bufsize)
        {
            bufsize += MAX_ARG_SIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens)
            {
                fprintf(stderr, "myshell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, " \t\r\n\a");
    }

    tokens[position] = NULL;
    return tokens;
}



// Execute commands for the user input
int executeCommand(char **args){
    pid_t pid, wpid;
    int status;

    if(args[0] == NULL){
        // An empty command was entered
        return 1;
    }
    // Handle exit command
    if(strcmp(args[0], "exit") == 0){
    return 0; // Exit command
    }

    // Handle cd command
    if(strcmp(args[0], "cd") == 0){
        // Execute cd command, but don't return its result
        char *newDir = cdCommand(args);
        if(newDir != NULL){
            free(newDir); // Free memory allocated to the directory
        }
        return 1; // continue running the shell
    }


    pid = fork();
    if(pid == 0){
        // Child process
        if(execvp(args[0], args) == -1){
            perror("myshell");
        } 
        exit(EXIT_FAILURE);
    } else if (pid < 0)
    {
        // Error forking
        perror("myshell");
    } else
    {
        // Parent process
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1; // Continue running shell
}
