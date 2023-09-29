#include <stdio.h>
#include <string.h>

int main() {
    char str[10] = "hello";
    strcpy(str, "z");
    printf("%s\n", str);
}