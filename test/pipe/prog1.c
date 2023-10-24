#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    write(1, "1\n", 2);
    exit(0);
}
