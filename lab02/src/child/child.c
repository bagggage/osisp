#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_LENGTH 256
#define ENV_COUNT 8
#define SLEEP_DELAY 5

void print_env(char **strings, size_t rows);
char **alloc_env_array(size_t rows, size_t columns);
void populate_child_env(char **child_env, char *child_env_path);

int main(int argc, char *argv[], char *envp[]) {
    int pid = getpid();
    int ppid = getppid();

    char *proc_name = argv[0];
    char *env_path = argv[1];

    printf("Process name: %s\nPID: %d\nPPID: %d\nEnvironment:\n", proc_name, pid, ppid);

    char **child_env = alloc_env_array(ENV_COUNT, MAX_LENGTH);

    populate_child_env(child_env, env_path);
    print_env(child_env, ENV_COUNT);

    sleep(SLEEP_DELAY);

    return 0;
}

char **alloc_env_array(size_t rows, size_t columns)
{
    char **result = calloc(rows, sizeof(char *));

    if (!result) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < rows; i++) {
        result[i] = calloc(columns, sizeof(char));

        if (!result[i]) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
    }

    return result;
}

void print_env(char **strings, size_t rows) {
    for (size_t i = 0; i < rows; i++) printf("%s\n", strings[i]);
}

void populate_child_env(char **child_env, char *child_env_path)
{
    FILE *env_file = fopen(child_env_path, "rt");

    if (env_file == NULL) {
        perror("Unable to open file");
        exit(EXIT_FAILURE);
    }

    char formatted[MAX_LENGTH];

    for (size_t i = 0; fscanf(env_file, "%s ", formatted) != EOF; i++) {
        sprintf(child_env[i], "%s=%s", formatted, getenv(formatted));
    }

    if (fclose(env_file) != 0) {
        perror("Unable to close file");
        exit(EXIT_FAILURE);
    }
}
