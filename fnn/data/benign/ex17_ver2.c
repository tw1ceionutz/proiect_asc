#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct ProcessNode {
    char* label;
    struct ProcessNode* left;
    struct ProcessNode* right;
} ProcessNode;

ProcessNode* parse_tree(const char** str);
void execute_tree(ProcessNode* node);
void free_tree(ProcessNode* node);
char* next_token(const char** str);

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
    if (node == NULL || strcmp(node->label, "@") == 0) return;

    pid_t pid = fork();
    if (pid == 0) {
        // Proces copil: Execută comanda
        execlp(node->label, node->label, (char*)NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }

    int status;
    waitpid(pid, &status, 0);

    // Afișează directorul curent
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Directorul curent: %s\n", cwd);
    } else {
        perror("getcwd");
    }

    // Continuă cu nodurile copil
    execute_tree(node->left);
    execute_tree(node->right);
}

void free_tree(ProcessNode* node) {
    if (node) {
        free_tree(node->left);
        free_tree(node->right);
        free(node->label);
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
    node->label = token;
    node->left = node->right = NULL;

    token = next_token(str);
    if (token && strcmp(token, "[") == 0) {
        free(token);
        node->left = parse_tree(str);
        
        token = next_token(str);
        if (token && strcmp(token, ",") == 0) {
            free(token);
            node->right = parse_tree(str);

            token = next_token(str);
            if (token && strcmp(token, "]") != 0) {
                free_tree(node);
                node = NULL;
            }
            free(token);
        }
    }
    return node;
}
