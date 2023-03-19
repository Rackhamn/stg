#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> // isatty()

#include <math.h>

// #include <GL/gl.h>
#include <GL/glew.h> // sudo apt-get install glew-utils
#include <SDL2/SDL.h>

#include "time.c"

/*
    opengl
        1. simple deffered shader
        2. VAO / VBO
        3. framebuffer target + texture
        4. render list
        5. upload textures
        6. render with uv or color
        7. build render buffer and then push it as the command
        8. add lighting
        9. add camera
        10. add scale, shear, translate, rotation
        
    models:
        triangle
        square / rectangle
        circle (approx)
        line

    collision / shapes:
        point
        line
        polyline
        square
        circle
        + swept


*/

const float identity_mat4[16] = 
  { 1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 
    0.0f, 0.0f, 0.0f, 1.0f };

typedef float vec3[3];

struct render_data_s {
    int vbo_count, vao_count;
    int * vbos, * vaos;

    int shader_count;
    int * shaders;

    // render type (LINE) -> vbo, vao, shader ids map 
};

// for timings
enum time_tag {
    TT_INIT = 0, 
    TT_DEINIT,
    TT_INPUT,
    TT_COMPUTE,
    TT_RENDER,
    TT_SLEEP,

    TT_MAX
};

// $ gcc main.c -o build/a.out -lSDL2 -lGL -lGLEW
int main(const int argc, const char ** argv) {

    int quit = 0;
    unsigned long long int start, end, elapsed; // us

    unsigned long long int frame_count;
    
    // runtime timings
    unsigned long long int total_timing[TT_MAX];

    unsigned long long int max_frame_time, sleep_time; 
    float target_fps, frame_delta_time;

    struct render_data_s render_data;

    SDL_Event sdl_event;
    SDL_version sdl_ver_compiled, sdl_ver_linked;

    SDL_Window * window;
	SDL_GLContext context;

    start = get_time_us();
    memset(total_timing, 0, TT_MAX * sizeof(unsigned long long int)); 

    // load default values:
    frame_count = 0;
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

    printf("\nrender_data_s\n");
    printf("* compile line shader\n");
    
    int line_vao, line_vbo, line_shader;

    {
        const char * line_vertex_shader_src = 
            "#version 330 core\n"
            "layout (location = 0) in vec3 pos;\n"
            "uniform mat4 mvp;\n"
            "void main() { \n"
            "\tgl_Position = mvp * vec4(pos, 1.0f);\n"
            "}\0";

        const char * line_fragment_shader_src = 
            "#version 330 core\n"
            "out vec4 fragcolor;\n"
            "uniform mat3 color;\n"
            "void main() { \n"
            "\tfragcolor = vec4(color, 1.0f);\n"
            "}\0";
    
        int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &line_vertex_shader_src, NULL);
        glCompileShader(vertex_shader);
        // check for errors etc

        int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &line_fragment_shader_src, NULL);
        glCompileShader(fragment_shader);

        // link to shader program
        int shader_program = glCreateProgram();
        glAttachShader(shader_program, vertex_shader);
        glAttachShader(shader_program, fragment_shader);
        glLinkProgram(shader_program);
        // check linkage error etc

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        line_shader = shader_program;

        // gen space
        glGenVertexArrays(1, &line_vao);
        glGenBuffers(1, &line_vbo);

        // bind for usage
        glBindVertexArray(line_vao);
        glBindBuffer(GL_ARRAY_BUFFER, line_vbo);

        // push data once
        float line_vertices[6] = { 0.1, 0.1, 1.0, 0.9, 0.9, 1.0 }; // two vec3 points  
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6, &line_vertices, GL_STATIC_DRAW);

        // setup how data is read and enable it
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // unbind 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        glUseProgram(0);        
    }

    end = get_time_us();
    elapsed = end - start;
    total_timing[TT_INIT] = elapsed;

    unsigned long long int frame_start, frame_end, frame_elapsed;
    while(!quit) {
        frame_start = get_time_us();

        start = frame_start;
        // handle input:
        while(SDL_PollEvent(&sdl_event)) {
		    switch(sdl_event.type) {
			    case SDL_QUIT:
				    printf("cmd: sdl_window_quit\n");
				    quit = 1;
			    break;
		    }
	    }
        end = get_time_us();
        total_timing[TT_INPUT] += end - start;

        start = end;
        // update:
        end = get_time_us();
        total_timing[TT_COMPUTE] += end - start;
        
        start = end;
        // render:
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        #if 0
        // opengl2 line
        glBegin(GL_LINES);
            glColor4f(0, 0, 1.0, 1.0);
            glVertex2f(.25, .25);
            glVertex2f(.75, .75);
        glEnd();
        #else
        // opengl3 line
        glUseProgram(line_shader);
        
        float * mvp = identity_mat4;
        vec3 line_color;

        line_color[0] = 0.0f;
        line_color[1] = 0.0f;
        line_color[2] = 1.0f;

        // build the mvp
        


        // push base data
        glUniformMatrix4fv(glGetUniformLocation(line_shader, "mvp"), 1, GL_FALSE, (const GLfloat *)&mvp);
        glUniform3fv(glGetUniformLocation(line_shader, "color"), 1, (const GLfloat *)&line_color[0]);
        
        glBindVertexArray(line_vao);

        glDrawArrays(GL_LINES, 0, 2);

        glBindVertexArray(0);
        glUseProgram(0);

        #endif

        SDL_GL_SwapWindow(window);

        end = get_time_us();
        total_timing[TT_RENDER] += end - start;

        frame_end = get_time_us();
        frame_elapsed = frame_end - frame_start;

        // sleep if there is time left!
        sleep_time = 0;        
        if(frame_elapsed >= max_frame_time) {
            // bad frame - overflow!
            printf("no time to sleep - overflow\n");
        } else {
            sleep_time = max_frame_time - frame_elapsed;
            if(sleep_time < max_frame_time) {
                sleep_us(sleep_time);
            } else {
                // no time left to sleep!
                printf("no time to sleep\n");
            }
        } 
        total_timing[TT_SLEEP] += sleep_time;

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
    total_timing[TT_DEINIT] = end - start;
    
    // runtime info:
    {
        // total runtime;
        unsigned long long int runtime = 0;
        unsigned long long int active_frame_time = 0;
        
        int i = 0;
        while(i < TT_MAX) {
            runtime += total_timing[i];
            i++;        
        }

        active_frame_time = total_timing[TT_INPUT] + total_timing[TT_COMPUTE] + total_timing[TT_RENDER];

        printf("\nruntime info:\n");
        printf("frame_count:   %llu\n", frame_count);
        printf("startup time:  %llu ms\n", total_timing[TT_INIT] / 1000);
        printf("cleanup time:  %llu ms\n", total_timing[TT_DEINIT] / 1000);

        printf("frame_timings\n");        
        printf("input time:    %llu ms\n", total_timing[TT_INPUT] / 1000);
        printf("update time:   %llu ms\n", total_timing[TT_COMPUTE] / 1000);
        printf("render time:   %llu ms\n", total_timing[TT_RENDER] / 1000);
        printf("sleep time:    %llu ms\n", total_timing[TT_SLEEP] / 1000);
        
        printf("\ntotal runtime: %llu ms\n", runtime / 1000); 
    }

    return 0;
}
