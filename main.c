#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> // isatty()

// #include <GL/gl.h>
#include <GL/glew.h> // sudo apt-get install glew-utils
#include <SDL2/SDL.h>

#include "time.c"

// $ gcc main.c -o build/a.out -lSDL2 -lGL -lGLEW
int main(const int argc, const char ** argv) {

    int quit = 0;
    unsigned long long int start, end, elapsed; // us

    unsigned long long int frame_count;
    unsigned long long int total_startup_time, total_cleanup_time, total_compute_time, total_sleep_time, total_run_time;

    unsigned long long int max_frame_time, sleep_time; 
    float target_fps, frame_delta_time;

    SDL_Event sdl_event;
    SDL_version sdl_ver_compiled, sdl_ver_linked;

    SDL_Window * window;
	SDL_GLContext context;

    start = get_time_us();

    // load default values:
    frame_count = 0;
    total_compute_time = 0;
    total_sleep_time = 0;
    total_run_time = 0;
    total_startup_time = 0;
    target_fps = 60.0f;
    
    // check if stdout is terminal or not (running from terminal) 
    if(!isatty(1)) {
        // use a log file
        // TODO: append filename with date / time?
        freopen("runtime.log", "w+", stdout);
    }

    // handle argv
    {
        int in_fps = 0;
        int i = 1;
        int arglen = 0;
        const char * arg = NULL;

        while(i < argc) {
            arg = argv[i];
            if(arg != NULL) {
                arglen = strlen(arg);
                if(memcmp(arg, "-fps=", 5)) {
                    // matches
                    in_fps = atoi(arg + 5);
                    if(in_fps > 0) {
                        printf("arg: fps = %d\n", in_fps);
                        target_fps = (float)in_fps;
                    } else {
                        printf("arg: [%s] value %d is not allowed\n", arg, in_fps);
                    }
                }
            }
            i++;
        }
    }

    // calc normal values:
    frame_delta_time = 1.0f / target_fps;
    max_frame_time = (unsigned long long int)(frame_delta_time * 1000 * 1000);

    

    // do rest of init:
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

    end = get_time_us();
    elapsed = end - start;
    total_startup_time = elapsed;

    while(!quit) {
        start = get_time_us();

        // handle input:
        while(SDL_PollEvent(&sdl_event)) {
		    switch(sdl_event.type) {
			    case SDL_QUIT:
				    printf("cmd: sdl_window_quit\n");
				    quit = 1;
			    break;
		    }
	    }

        // update:
        
        // render:
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        SDL_GL_SwapWindow(window);
    
        end = get_time_us();
        elapsed = end - start;
        total_compute_time += elapsed;

        // sleep if there is time left!
        sleep_time = 0;        
        if(elapsed >= max_frame_time) {
            // bad frame - overflow!
            printf("no time to sleep - overflow\n");
        } else {
            sleep_time = max_frame_time - elapsed;
            if(sleep_time < max_frame_time) {
                sleep_us(sleep_time);
            } else {
                // no time left to sleep!
                printf("no time to sleep\n");
            }
        } 
        total_sleep_time += sleep_time;

        frame_count += 1;
    }

    start = get_time_us();
    printf("Destroy GL context\n");
    SDL_GL_DeleteContext(context);

    printf("Destroy SDL windows\n");
    SDL_DestroyWindow(window);

    printf("Quit SDL\n");
    SDL_Quit();    
    end = get_time_us();
    total_cleanup_time = end - start;
    
    // runtime info:
    {
        printf("\nruntime info:\n");
        printf("frame_count: %llu\n", frame_count);
        printf("startup time: %llu ms\n", total_startup_time / 1000);
        printf("cleanup time: %llu ms\n", total_cleanup_time / 1000);
        printf("compute time: %llu ms\n", total_compute_time / 1000);
        printf("sleep time:   %llu us\n", total_sleep_time / 1000);
        
        unsigned long long int runtime = total_startup_time + total_compute_time + total_sleep_time; // us
        printf("runtime:      %llu ms\n", runtime / 1000); 
    }

    return 0;
}
