#include <stdlib.h>
#include <stdio.h>

#include <SDL2/SDL.h>

#include "time.c"

// $ gcc main.c -o a.out -lSDL2
int main(int argc, char ** argv) {

    int quit = 0;
    unsigned long long int start, end, elapsed; // us 

    SDL_Event sdl_event;
    SDL_version sdl_ver_compiled, sdl_ver_linked;

    start = get_time_us();

    printf("hello world!\n");

    printf("init SDL\n");
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_VERSION(&sdl_ver_compiled);
    SDL_GetVersion(&sdl_ver_linked);

    printf("* compiled against SDL version %u.%u.%u\n", 
            sdl_ver_compiled.major, sdl_ver_compiled.minor, sdl_ver_compiled.patch);
    printf("* linked against SDL version %u.%u.%u\n", 
            sdl_ver_linked.major, sdl_ver_linked.minor, sdl_ver_linked.patch);

    sleep_us(1000);

    printf("Quit SDL\n");
    SDL_Quit();    

    end = get_time_us();
    elapsed = end - start;

    printf("runtime: %llu (us)\n", elapsed);

    return 0;
}
