#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>

char *realpath_custom(const char *path) {
    char *result = malloc(PATH_MAX);
    if (result == NULL) {
        perror("malloc failed");
        return NULL;
    }
    memset(result, 0, PATH_MAX); // Inițializarea corectă a bufferului result

    // Verificăm dacă calea este deja absolută
    if (path[0] == '/') {
        strncpy(result, path, PATH_MAX);
    } else {
        // Obținem calea absolută curentă și adăugăm calea specificată
        if (getcwd(result, PATH_MAX) == NULL) {
            perror("getcwd failed");
            free(result);
            return NULL;
        }
        strncat(result, "/", PATH_MAX - strlen(result) - 1);
        strncat(result, path, PATH_MAX - strlen(result) - 1);
    }

    // Realizăm normalizarea căii
    char *real_path = realpath(result, NULL);
    if (real_path == NULL) {
        perror("realpath failed");
        free(result);
        return NULL;
    }

    free(result);
    return real_path;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *abs_path = realpath_custom(argv[1]);
    if (abs_path == NULL) {
        return EXIT_FAILURE;
    }

    printf("Realpath: %s\n", abs_path);
    free(abs_path);

    return EXIT_SUCCESS;
}

