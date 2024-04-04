#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/wait.h>

#define MAX_LENGTH 256
#define CHILD_NAME_LENGTH 9
#define ENV_COUNT 11

extern char **environ;

void sort_env(char ***strings, size_t rows);
void print_env(char **strings, size_t rows);

char *find_env(char **env, char *key, size_t rows);

int fork_proc(char* child_path, char** child_args);

int main(int argc, char *argv[], char *envp[]) {
    if (argc < 2) {
        fprintf(stderr, "Error! File is not provided\n");
        exit(EXIT_FAILURE);
    }

    size_t env_size = 0;

    while (environ[env_size] != NULL) {
        env_size++;
    }

    sort_env(&environ, env_size);
    print_env(environ, env_size);

    char *child_env_path = argv[1];
    size_t child_idx = 0;

    while (1) {
        setenv("LC_COLLATE", "C", 1);
        printf("\nEnter operation: \'+\', \'*\', \'&\', q(quit).\n>");
        rewind(stdin);

        char opt;

        while (1) {
            if (scanf("%c", &opt) < 1) continue;

            if (opt == '+' || opt == '*' || opt == '&' || opt == 'q') {
                break;
            }
        }

        char child_name[CHILD_NAME_LENGTH];
        snprintf(child_name, CHILD_NAME_LENGTH, "%s%02d", "child_", (int)child_idx);

        int status_of_child = 0;
        char *child_args[] = { child_name, child_env_path, NULL };

        switch (opt) {
        case '+':
            fork_proc(getenv("CHILD_PATH"), child_args);
            wait(&status_of_child);
            break;
        case '*':
            fork_proc(find_env(envp, "CHILD_PATH", env_size), child_args);
            wait(&status_of_child);
            break;
        case '&':
            fork_proc(find_env(environ, "CHILD_PATH", env_size), child_args);
            wait(&status_of_child);
            break;
        case 'q':
            exit(EXIT_SUCCESS);
        default:
            break;
        }

        child_idx++;

        if (child_idx > 99) child_idx = 0;
    }
}

int fork_proc(char* child_path, char** child_args) {
    int pid = fork();

    if (pid == -1) {
        printf("Error occurred, code - %d\n", errno);
        exit(errno);
    }
    else if (pid == 0) {
        printf("Child proc created...\n");
        execve(child_path, child_args, environ);
    }

    return pid;
}

void print_env(char **strings, size_t rows) {
    for (size_t i = 0; i < rows; i++) fprintf(stdout, "%s\n", strings[i]);
}

void swap(char **s1, char **s2) {
    char *tmp = *s1;
    *s1 = *s2;
    *s2 = tmp;
}

void sort_env(char ***strings, size_t rows) {
    for (size_t i = 0; i < rows - 1; i++) {
        for (size_t j = 0; j < rows - i - 1; j++) {
            if (strcoll((*strings)[j], (*strings)[j + 1]) > 0) {
                swap(&((*strings)[j]), &((*strings)[j + 1]));
            }
        }
    }
}

char* find_env(char **env, char *key, size_t rows) {
    char *result = calloc(MAX_LENGTH, sizeof(char));

    for (size_t i = 0; i < rows; i++) {
        if (strstr(env[i], key)) strncpy(result, env[i] + strlen(key) + 1, MAX_LENGTH);
    }

    return result;
}