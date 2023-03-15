#include <stdlib.h>
#include <stdio.h>

#include "time.c"

// $ gcc main.c -o a.out
int main(int argc, char ** argv) {

    unsigned long long int start, end, elapsed; // us 

    start = get_time_us();

    printf("hello world!\n");

    end = get_time_us();
    elapsed = end - start;

    printf("runtime: %llu (us)\n", elapsed);

    return 0;
}
