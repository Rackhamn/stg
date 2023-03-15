#include <stdlib.h>
#include <stdio.h>

#include "time.c"

// $ gcc main.c -o a.out
int main(int argc, char ** argv) {

    unsigned long long int start, end, elapsed; // us 

    start = get_time_ns();

    printf("hello world!\n");

    end = get_time_ns();
    elapsed = end - start;

    printf("runtime: %llu (ns)\n", elapsed);

    return 0;
}
