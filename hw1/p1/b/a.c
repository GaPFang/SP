#include <stdio.h>
#include <stdlib.h>

int main() {
    char *s = malloc(sizeof(char) * 20);
    scanf("%s", s);
    printf("%s", s);
    return 0;
}