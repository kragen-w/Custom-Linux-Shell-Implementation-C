#include <stdio.h>

int main() {
    char input[100];

    printf("Enter some input: ");
    
    // Use scanf to read from stdin (which will be redirected)
    if (scanf("%s", input) == 1) {
        printf("You entered: %s\n", input);
    } else {
        printf("Failed to read input.\n");
    }

    return 0;
}