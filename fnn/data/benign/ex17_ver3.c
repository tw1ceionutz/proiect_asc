#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_ARGS 10

typedef struct ProcessNode {
    char** command;
    struct ProcessNode* left;
    struct ProcessNode* right;
} ProcessNode;

ProcessNode* parse_tree(const char** str);
void execute_tree(ProcessNode* node);
void free_tree(ProcessNode* node);
char* next_token(const char** str);
char** split_command(const char* command);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s 'expression'\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* input = argv[1];
    ProcessNode* root = parse_tree(&input);
    execute_tree(root);
    free_tree(root);

    return EXIT_SUCCESS;
}

void execute_tree(ProcessNode* node) {
    if (node == NULL) return;

    pid_t pid = fork();
    if (pid == 0) {
        execvp(node->command[0], node->command);
        perror("execvp");
        exit(EXIT_FAILURE);
    }

    int status;
    waitpid(pid, &status, 0);

    execute_tree(node->left);
    execute_tree(node->right);
}

void free_tree(ProcessNode* node) {
    if (node) {
        free_tree(node->left);
        free_tree(node->right);
        for (int i = 0; node->command[i] != NULL; i++) {
            free(node->command[i]);
        }
        free(node->command);
        free(node);
    }
}

char* next_token(const char** str) {
    while (**str == ' ') (*str)++;

    if (**str == '@' || **str == '[' || **str == ']' || **str == ',') {
        return strndup((*str)++, 1);
    }

    const char* start = *str;
    while (**str && **str != ' ' && **str != '[' && **str != ']' && **str != ',') (*str)++;
    return strndup(start, *str - start);

}

ProcessNode* parse_tree(const char** str) {
    char* token = next_token(str);
    if (token == NULL) return NULL;

    if (strcmp(token, "@") == 0) {
        free(token);
        return NULL;
    }

    ProcessNode* node = malloc(sizeof(ProcessNode));
    node->command = split_command(token);
    free(token);
    node->left = node->right = NULL;
    token = next_token(str);
     if (token && strcmp(token, "[") == 0) {
        free(token);
        node->left = parse_tree(str);
        token = next_token(str);  // obține următorul token după nodul stâng

        if (token && strcmp(token, ",") == 0) {
            free(token);
            node->right = parse_tree(str);
            token = next_token(str);  // obține următorul token după nodul drept
        }
    }

    // Verificăm dacă tokenul este "]" la sfârșitul listei de noduri
    if (token && strcmp(token, "]") == 0) {
        free(token);
    } else {
        // Eliberăm arborele în caz de structură incorectă și returnăm NULL
        free_tree(node);
        return NULL;
    }

    return node;
}

char** split_command(const char* command) {
    char** args = malloc(sizeof(char*) * (MAX_ARGS + 1));
    int i = 0;
    const char* start = command;
    while (*command) {
        if (*command == ' ' || *(command + 1) == '\0') {
            int len = command - start + (*command != ' ');
            args[i] = strndup(start, len);
            i++;
            if (i >= MAX_ARGS) break;  // Prevenim depășirea numărului maxim de argumente
            start = command + 1;
        }
        command++;
    }
    args[i] = NULL;
    return args;
}
