#include <stdio.h>
#include <unistd.h>

int main() {
    printf("Process 3 with pid = %d\n", getpid());
    return 0;
}