#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 100
#define MAX_ARGS 64

char *trim(char *str) {
    while (*str == ' ' || *str == '\t') str++;
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t')) *end-- = '\0';
    return str;
}


void split_commands(char *line, char **commands, int *count) {
    *count = 0;
    char *start = line;
    int in_quotes = 0;
    char quote_char = 0;

    for (char *p = line; ; ++p) {
        if (*p == '"' || *p == '\'') {
            if (!in_quotes) {
                in_quotes = 1;
                quote_char = *p;
            } else if (quote_char == *p) {
                in_quotes = 0;
            }
        } else if (*p == '|' && !in_quotes) {
            *p = '\0';
            commands[(*count)++] = trim(start);
            start = p + 1;
        } else if (*p == '\0') {
            commands[(*count)++] = trim(start);
            break;
        }
    }
}


int parse_args(char *cmd, char **args) {
    int i = 0;
    char *p = cmd;

    while (*p) {
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') break;

        if (*p == '"' || *p == '\'') {
            char quote = *p++;
            if (i >= MAX_ARGS) {
                fprintf(stderr, "Error: demasiados argumentos\n");
                return -1;
            }
            args[i++] = p;
            while (*p && *p != quote) p++;
            if (*p == '\0') {
                fprintf(stderr, "Error: comillas sin cerrar\n");
                return -1;
            }
            *p = '\0';
            p++;
        } else {
            if (i >= MAX_ARGS) {
                fprintf(stderr, "Error: demasiados argumentos\n");
                return -1;
            }
            args[i++] = p;
            while (*p && *p != ' ' && *p != '\t') p++;
            if (*p) {
                *p = '\0';
                p++;
            }
        }
    }
    args[i] = NULL;
    return i;
}

int main() {
    char command[1024];
    char *commands[MAX_COMMANDS];

    while (1) {
        if (isatty(STDIN_FILENO)) {
            printf("Shell> ");
            fflush(stdout);
        }

        if (!fgets(command, sizeof(command), stdin))
            break;

        command[strcspn(command, "\n")] = '\0';

        char *line = trim(command);

        if (strcmp(line, "exit") == 0) {
            break;
        }

        if (line[0] == '|' || line[strlen(line) - 1] == '|') {
            fprintf(stderr, "Error: pipe al inicio o final\n");
            continue;
        }

        if (strstr(line, "||") != NULL) {
            fprintf(stderr, "Error: pipes dobles no permitidos\n");
            continue;
        }

        int command_count = 0;
        split_commands(line, commands, &command_count);

       
        int error_pipe = 0;
        for (int i = 0; i < command_count; i++) {
            if (strlen(commands[i]) == 0) {
                error_pipe = 1;
                break;
            }
        }
        if (error_pipe) {
            fprintf(stderr, "Error: comando vacío en pipeline\n");
            continue;
        }

        int prev_fd = -1;
        int pipefd[2];

        for (int i = 0; i < command_count; i++) {
            int use_pipe = (i < command_count - 1);
            if (use_pipe) {
                if (pipe(pipefd) == -1) {
                    perror("pipe");
                    exit(1);
                }
            }

            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                exit(1);
            }

            if (pid == 0) {
                if (i > 0) {
                    dup2(prev_fd, STDIN_FILENO);
                    close(prev_fd);
                }
                if (use_pipe) {
                    dup2(pipefd[1], STDOUT_FILENO);
                }

                if (use_pipe) close(pipefd[0]);
                if (use_pipe) close(pipefd[1]);

                char *args[MAX_ARGS + 1];
                int arg_count = parse_args(commands[i], args);

                if (arg_count == -1 || args[0] == NULL) {
                    fprintf(stderr, "Error: comando vacío\n");
                    exit(1);
                }
                if (arg_count > MAX_ARGS) {
                    fprintf(stderr, "Error: demasiados argumentos\n");
                    exit(1);
                }

                execvp(args[0], args);
                perror("execvp");
                exit(1);
            }

            if (prev_fd != -1)
                close(prev_fd);
            if (use_pipe) {
                close(pipefd[1]);
                prev_fd = pipefd[0];
            }
        }

        
        if (prev_fd != -1)
            close(prev_fd);

        while (wait(NULL) > 0)
            ;
    }

    return 0;
}