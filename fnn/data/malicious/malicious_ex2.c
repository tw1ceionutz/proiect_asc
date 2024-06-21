// malicious_delete_file.c
#include <stdio.h>

int main() {
    remove("/home/ionutz/important_file.txt");
    return 0;
}
