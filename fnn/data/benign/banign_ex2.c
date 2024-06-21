// benign_calculator.c
#include <stdio.h>

int main() {
    int a, b;
    char op;
    printf("Enter an expression (e.g., 3 + 4): ");
    scanf("%d %c %d", &a, &op, &b);

    switch (op) {
        case '+': printf("Result: %d\n", a + b); break;
        case '-': printf("Result: %d\n", a - b); break;
        case '*': printf("Result: %d\n", a * b); break;
        case '/': printf("Result: %d\n", a / b); break;
        default: printf("Unknown operator\n");
    }
    return 0;
}
