#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// Creating empty functions before including them in the command_func array
int ok_cd(char **args);
int ok_exit(char **args);
int ok_nmd(char **args);

// Creating an array containing command names
char *command_names[] = {
    "cd",
    "exit",
    "nmd"
};

// Creating an array containing the functions themselves
int (*command_func[]) (char **) = {
    &ok_cd,
    &ok_exit,
    &ok_nmd
};

// A function that's called when user enters the "cd" command
int ok_cd(char **args) {
    chdir(args[1]);
    return 1;
}

// When user enters "exit"
int ok_exit(char **args) {
    exit(EXIT_SUCCESS);
}

// When user enters "nmd" (not markdown) and new folder name. Creates folder and changes into it
int ok_nmd(char **args) {
    pid_t pid;
    int status;
    char *thecmd[] = {"mkdir", args[1], NULL};

    pid = fork();
    if (pid == 0) {
        execvp(thecmd[0], thecmd); // execvp() is how you run unix commands except for cd
    } else {
        waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));

    chdir(args[1]);
    return 1;
}

#define TOKEN_BUFSIZE 64
#define TOKEN_DELIM " \t\r\n\a"

// Read the line and split it into arguments by using predefined delimiters/separators
char **ok_get_args(void) {
    char *line = NULL;
    ssize_t bufsize = 0;

    printf("< ");
    getline(&line, &bufsize, stdin);

    bufsize = TOKEN_BUFSIZE;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    token = strtok(line, TOKEN_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;
        token = strtok(NULL, TOKEN_DELIM); // This is how you keep tokenizing from where you last left off
    }
    tokens[position] = NULL;
    return tokens;
}

int ok_execute(char **args) {
    int i;
    pid_t pid;
    int status;

    if (args[0] == NULL) {
        return 1;
    }

    // If command is in the predefined array then run the corresponding function
    for (i = 0; i < (sizeof(command_names) / sizeof(char*)); i++) {
        if (strcmp(args[0], command_names[i]) == 0) {
            return (*command_func[i])(args);
        }
    }

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) { // Gives error if something other than a command was entered
            perror("okcshell");
        }
        exit(EXIT_FAILURE);
    } else {
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

int main() {
    char **args;
    do {
        args = ok_get_args();
        ok_execute(args);
        free(args);
    } while (1);
    return 0;
}