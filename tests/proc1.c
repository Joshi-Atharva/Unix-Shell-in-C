#include <stdio.h>
#include <unistd.h>
int main() { 
    printf("Process 1 with pid = %d\n", getpid());
    return 0;
}