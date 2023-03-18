#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> // isatty()

// #include <GL/gl.h>
#include <GL/glew.h> // sudo apt-get install glew-utils
#include <SDL2/SDL.h>

#include "time.c"

// $ gcc main.c -o build/a.out -lSDL2 -lGL -lGLEW
int main(int argc, char ** argv) {

    int quit = 0;
    unsigned long long int start, end, elapsed; // us 

    SDL_Event sdl_event;
    SDL_version sdl_ver_compiled, sdl_ver_linked;

    SDL_Window * window;
	SDL_GLContext context;

    start = get_time_us();

    // check if stdout is terminal or not
    if(isatty(1)) {
        // using terminal
    } else {
        // use a log file
        freopen("runtime.log", "a+", stdout);
    }

    printf("hello world!\n");

    printf("init SDL\n");
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_VERSION(&sdl_ver_compiled);
    SDL_GetVersion(&sdl_ver_linked);

    printf("* compiled against SDL version %u.%u.%u\n", 
            sdl_ver_compiled.major, sdl_ver_compiled.minor, sdl_ver_compiled.patch);

    printf("* linked against SDL version %u.%u.%u\n", 
            sdl_ver_linked.major, sdl_ver_linked.minor, sdl_ver_linked.patch);

    printf("create SDL window\n");
    unsigned int window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    window = SDL_CreateWindow("title", 
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            640, 480, window_flags); 

    printf("create GL context\n");
    context = SDL_GL_CreateContext(window);
    // v-sync with monitor refresh rate
	SDL_GL_SetSwapInterval(0); // disable vsync for N-fps

    // set GL attributes for api	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // init glew (gl bindings)
    glewInit();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // red backgroud
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

    while(!quit) {
        while(SDL_PollEvent(&sdl_event)) {
		    switch(sdl_event.type) {
			    case SDL_QUIT:
				    printf("cmd: sdl_window_quit\n");
				    quit = 1;
			    break;
		    }
	    }

        // sleep_us(1000);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        SDL_GL_SwapWindow(window);
    }

    printf("Destroy GL context\n");
    SDL_GL_DeleteContext(context);

    printf("Destroy SDL windows\n");
    SDL_DestroyWindow(window);

    printf("Quit SDL\n");
    SDL_Quit();    

    end = get_time_us();
    elapsed = end - start;

    printf("runtime: %llu (us)\n", elapsed);

    return 0;
}
