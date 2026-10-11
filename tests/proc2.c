#include <stdio.h>
#include <unistd.h>
int main(int argc, char *argv[]) {
    printf("Process 2 with pid = %d\n", getpid());
    return 0;
}