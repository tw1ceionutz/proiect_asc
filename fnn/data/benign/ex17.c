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

// Prototipuri de funcții
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

    int left_pipe[2];
    int right_pipe[2];
    char buffer[1024];
    ssize_t nbytes;

    if (pipe(left_pipe) == -1 || pipe(right_pipe) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t left_pid = fork();
    if (left_pid == 0) { // Copilul stâng
        close(left_pipe[0]); // Închide citirea deoarece nu este necesară
        dup2(left_pipe[1], STDOUT_FILENO); // Redirecționează stdout la pipe
        dup2(left_pipe[1], STDERR_FILENO); // Redirecționează stderr la pipe
        close(left_pipe[1]);

        execute_tree(node->left);
        exit(0);
    }

    pid_t right_pid = fork();
    if (right_pid == 0) { // Copilul drept
        close(right_pipe[0]);
        dup2(right_pipe[1], STDOUT_FILENO);
        dup2(right_pipe[1], STDERR_FILENO);
        close(right_pipe[1]);

        execute_tree(node->right);
        exit(0);
    }

    // Procesul părinte
    close(left_pipe[1]);
    close(right_pipe[1]);

    while ((nbytes = read(left_pipe[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[nbytes] = '\0';
        printf("Output from left child: %s", buffer);
    }

    while ((nbytes = read(right_pipe[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[nbytes] = '\0';
        printf("Output from right child: %s", buffer);
    }

    close(left_pipe[0]);
    close(right_pipe[0]);

    waitpid(left_pid, NULL, 0);
    waitpid(right_pid, NULL, 0);

    if (strcmp(node->label, "@") != 0) {
        // Execută comanda pentru procesul curent și afișează rezultatul
        int pfd[2];
        pipe(pfd);
        pid_t pid = fork();
        if (pid == 0) {
            close(pfd[0]);
            dup2(pfd[1], STDOUT_FILENO);
            dup2(pfd[1], STDERR_FILENO);
            close(pfd[1]);
            execlp(node->label, node->label, (char*)NULL);
            perror("execlp");
            exit(EXIT_FAILURE);
        } else {
            close(pfd[1]);
            while ((nbytes = read(pfd[0], buffer, sizeof(buffer) - 1)) > 0) {
                buffer[nbytes] = '\0';
                printf("Output from node %s: %s", node->label, buffer);
            }
            close(pfd[0]);
            waitpid(pid, NULL, 0);
        }
    }
}

// Funcția de eliberare a memoriei pentru arbore
void free_tree(ProcessNode* node) {
    if (node) {
        free_tree(node->left);
        free_tree(node->right);
        free(node->label);
        free(node);
    }
}
// Funcția next_token parsează și returnează următorul token din șirul dat
char* next_token(const char** str) {
    while (**str == ' ') (*str)++;  // Sărim peste spațiile albe

    if (**str == '@' || **str == '[' || **str == ']' || **str == ',') {
        return strndup((*str)++, 1);  // Returnăm simbolul ca un nou șir
    }

    const char* start = *str;
    while (**str && **str != ' ' && **str != '[' && **str != ']' && **str != ',') (*str)++;
    return strndup(start, *str - start);  // Returnăm cuvântul ca un nou șir
}

ProcessNode* parse_tree(const char** str) {
    char* token = next_token(str);
    if (token == NULL) return NULL;

    if (strcmp(token, "@") == 0) {
        free(token);
        return NULL; // Arborele vid
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


