#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> // isatty()

#include <math.h>

// #include <GL/gl.h>
#include <GL/glew.h> // sudo apt-get install glew-utils
#include <SDL2/SDL.h>

#include "time.c"

#include "mat4.h"

#define A2R		(0.01745329252f)

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

struct shader {
    int id;
    char * tag;
    
    // map:
    int loc_count;
    char * loc_tags;    // key
    GLuint * locs;      // value
};

// $ gcc main.c -o build/a.out -lm -lSDL2 -lGL -lGLEW
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
    SDL_GL_SetAttribute (SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16); // 24

    // init glew (gl bindings)
    glewInit();
	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    printf ("glGetString (GL_VERSION) returns %s\n", glGetString (GL_VERSION));

    // actually situationally dependant (diff between see-through models)
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

	glEnable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, 640, 480);

    // red backgroud
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

    printf("* compile line shader\n");
    
    GLuint line_vao, line_vbo, line_shader;
    GLuint line_shader_mvp_loc, proj_loc, view_loc; 
    GLuint line_shader_color_loc;

    {
        // does not work on my pc -> needs newer opengl version
        #if 0
        const char * line_vertex_shader_src = 
            "#version 330 core\n"
            "layout (location = 0) in vec3 pos;\n"
            "uniform mat4 proj;\n"
            "uniform mat4 view;\n"            
            "void main() { \n"
            "\tmat4 mvp = view * proj;\n"
            "\tgl_Position = mvp * vec4(pos, 1.0f);\n"
            "}\0";
            // "uniform mat4 mvp;\n"

        const char * line_fragment_shader_src = 
            "#version 330 core\n"
            "out vec4 fragcolor;\n"
            "uniform vec3 color;\n"
            "void main() { \n"
            "\tfragcolor = vec4(color, 1.0f);\n"
            "}\0";
        #else

        const char * line_vertex_shader_src = 
            "#version 130\n"
            "uniform mat4 proj;\n"
            "uniform mat4 view;\n"            
            "uniform mat4 mvp;\n"
            "in vec3 pos;\n"
            "void main() {\n"
            // "\tmat4 mvp = proj * view;\n"
            "\tgl_Position = mvp * vec4(pos, 1.0f);\n"
            "}\0";

        const char * line_fragment_shader_src = 
            "#version 130\n"
            "uniform vec3 color;\n"
            "out vec4 fragcolor;\n"
            "void main() {\n"
            "\tfragcolor = vec4(color, 1.0f);\n"
            "}\0";
        #endif
    
        int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &line_vertex_shader_src, NULL);
        glCompileShader(vertex_shader);
        // check for errors etc
        GLint status;
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE )
        {
            char buf[1024];
            GLint logLen;
            glGetShaderiv( vertex_shader, GL_INFO_LOG_LENGTH, &logLen );
            GLsizei written;
            glGetShaderInfoLog( vertex_shader, logLen, &written, buf);
            printf("vs: comp err: %s\n", buf);
        }

        int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &line_fragment_shader_src, NULL);
        glCompileShader(fragment_shader);

        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status);
        if(status == GL_FALSE )
        {
            char buf[1024];
            GLint logLen;
            glGetShaderiv( fragment_shader, GL_INFO_LOG_LENGTH, &logLen );
            GLsizei written;
            glGetShaderInfoLog( fragment_shader, logLen, &written, buf );
            printf("fs: comp err: %s\n", buf);
        }


        // link to shader program
        int shader_program = glCreateProgram();
        // glBindAttribLocation(shader_program, 0, "pos");

        glAttachShader(shader_program, vertex_shader);
        glAttachShader(shader_program, fragment_shader);
        glLinkProgram(shader_program);
        // check linkage error etc
        glGetProgramiv( shader_program, GL_LINK_STATUS, &status );
        if ( status == GL_FALSE )
        {
            char buf[1024];
            GLint logLen;
            glGetProgramiv( shader_program, GL_INFO_LOG_LENGTH, &logLen );
            GLsizei written;
            glGetProgramInfoLog( shader_program, logLen, &written, buf );
            printf("link err: %s\n", buf);
        }
        glValidateProgram(shader_program);
        glGetProgramiv(shader_program, GL_VALIDATE_STATUS, &status);
        if (!status) {
            char buf[1024];
            GLint logLen;
            glGetProgramiv( shader_program, GL_INFO_LOG_LENGTH, &logLen );
            GLsizei written;
            glGetProgramInfoLog( shader_program, logLen, &written, buf );
            printf("link valid err: %s\n", buf);
        }

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        line_shader = shader_program;
        glUseProgram(line_shader);

        // gen space
        glGenVertexArrays(1, &line_vao);
        glGenBuffers(1, &line_vbo);

        // bind for usage
        glBindVertexArray(line_vao);
        glBindBuffer(GL_ARRAY_BUFFER, line_vbo);

        // push data once
        float vertices[] = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.0f,  0.5f, 0.0f
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 9, &vertices, GL_STATIC_DRAW);

        // setup how data is read and enable it
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        line_shader_mvp_loc = glGetUniformLocation(line_shader, "mvp");
        line_shader_color_loc = glGetUniformLocation(line_shader, "color");

        proj_loc = glGetUniformLocation(line_shader, "proj");
        view_loc = glGetUniformLocation(line_shader, "view");

        // unbind 
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);

        printf("shader comp complete\n");
    }

    end = get_time_us();
    elapsed = end - start;
    total_timing[TT_INIT] = elapsed;

    // cam movement
    float dx = 0.0f;
    float dy = 1.0f;
    float c_force_x = 8.0f;
    float c_force_y = 8.0f;
    float vel_x, vel_y;

    // TODO: remappable keys
    // TODO: keylogger and replayer 
    // TODO: separete modules for the remapper and keylogger (always live but only fires if needed)
    // TODO: add copy of the map during mapping so that you then can APPLY or DISCARD changes. (can also mask changes)

    struct input_key {
        // int tag; // action key map
        long int keysym;
        int value;
    };

    struct index_map {
        int from;
        int to;
    };

    struct input_action_key {
        int tag;
        union {
            int i;
            float f;
        } value;
    };

    int is_remapping = 0;
    int im_index = -1;
    struct index_map c_im_kb;

    int                     in_kb[SDL_NUM_SCANCODES]; 
    struct index_map        im_kb[4];
    struct input_action_key iak[4]; // keep one per frame

    int                     in_kb_prev[SDL_NUM_SCANCODES];
    struct index_map        im_kb_prev[4];
    struct input_action_key iak_prev[4]; 

    memset(in_kb, 0, sizeof(int) * SDL_NUM_SCANCODES);
    memset(in_kb_prev, 0, sizeof(int) * SDL_NUM_SCANCODES);

    im_kb[0].from = SDL_SCANCODE_LEFT;
    im_kb[0].to = 0;
    im_kb[1].from = SDL_SCANCODE_RIGHT;
    im_kb[1].to = 1;
    im_kb[2].from = SDL_SCANCODE_DOWN;
    im_kb[2].to = 2;
    im_kb[3].from = SDL_SCANCODE_UP;
    im_kb[3].to = 3;

    c_im_kb.from = -1;
    c_im_kb.to = -1;

    memset(iak, 0, sizeof(struct input_action_key) * 4);
    memset(iak_prev, 0, sizeof(struct input_action_key) * 4);
    iak[0].tag = 0;
    iak[1].tag = 1;
    iak[2].tag = 2;
    iak[3].tag = 3;

    unsigned long long int frame_start, frame_end, frame_elapsed;

    long int scancode;
    long int keysym;
    
    while(!quit) {
        frame_start = get_time_us();
    
        // ok        
        vel_x = 0.0f;
        vel_y = 0.0f;

        memcpy(in_kb_prev, in_kb, sizeof(int) * SDL_NUM_SCANCODES);
        memcpy(iak_prev, iak, sizeof(struct input_action_key) * 4);

        start = frame_start;
        // read input:
        while(SDL_PollEvent(&sdl_event) != 0) {
		    switch(sdl_event.type) {
                 case SDL_KEYDOWN: {
                    scancode = sdl_event.key.keysym.scancode;
                    in_kb[scancode] = 1;
                } break;
                case SDL_KEYUP: {
                    scancode = sdl_event.key.keysym.scancode;
                    in_kb[scancode] = 0;
                } break;
			    case SDL_QUIT: {
				    printf("cmd: sdl_window_quit\n");
				    quit = 1;
			    } break;
		    }
	    }
        
        // TODO: move mapping to separete module!
        if(in_kb[SDL_SCANCODE_Q] && !in_kb_prev[SDL_SCANCODE_Q]) { // hacky inital check
            is_remapping = !is_remapping;
            if(is_remapping) {
                printf("start remapping\n");
                c_im_kb.from = -1;
                c_im_kb.to = -1;
                im_index = -1;
            } else {
                printf("stopped remapping\n");
                c_im_kb.from = -1;
                c_im_kb.to = -1;
                im_index = -1;
            }
        }

        if(is_remapping && !in_kb[SDL_SCANCODE_Q]) {
            const char * scancode_name = NULL;
            
            if(im_index == -1) {
                // await scancode to edit
                
                // find first key pressed this frame using scancode
                for(int i = 0; i < SDL_NUM_SCANCODES; i++) {
                    if(in_kb[i] && !in_kb_prev[i]) {
                        
                        scancode_name = SDL_GetScancodeName(i);
                        printf("check scancode: %i, %s\n", i, scancode_name); 
                        
                        // find if scancode is actually used in current mapping
                        for(int j = 0; j < 4; j++) {
                            if(im_kb[j].from == i) {
                                printf("found mapping at %i for given scancode %i, %s\n", j, i, scancode_name);
                                im_index = j;
                                break;
                            } 
                        }

                        if(im_index == -1)
                            printf("scancode %i, %s is not used in any mappings\n", i, scancode_name);
        
                    }               
                }
            } else {
                // await scancode to swap to

                // find first key pressed this frame using scancode
                for(int i = 0; i < SDL_NUM_SCANCODES; i++) {
                    if(in_kb[i] && !in_kb_prev[i]) {

                        const char * prev_scancode_name = SDL_GetScancodeName(im_kb[im_index].from);
                        scancode_name = SDL_GetScancodeName(i);

                        printf("changed scancode %i, %s to %i, %s\n", 
                                                im_kb[im_index].from, prev_scancode_name, 
                                                i, scancode_name);
                        
                        im_kb[im_index].from = i;

                        // TODO: handle multiple mappings using the same key -> removal
                        for(int j = 0; j < 4; j++) {
                            if(j != im_index) {
                                if(im_kb[j].from == i) {
                                    // mapping already existed. prioritise the new mapping and remove the previus one.
                                    im_kb[j].from = -1;
                                }
                            }
                        }

                        im_index = -1;
                        break;
                    }
                }
            }
        }
        else {
            // use the mapping to read stuff 
            struct index_map _im;
            for(int i = 0; i < 4; i++) {
                _im = im_kb[i];
                if(_im.from > -1 && _im.to > -1) {
                    iak[_im.to].value.i = in_kb[_im.from];
                } else {
                    // something is wrong with the mapping, clear to 0
                    if(_im.to > -1) iak[_im.to].value.i = 0;
                }
            }

            if(iak[0].value.i) { vel_x -= 1.0f; }
            if(iak[1].value.i) { vel_x += 1.0f; }
            if(iak[2].value.i) { vel_y += 1.0f; }
            if(iak[3].value.i) { vel_y -= 1.0f; }
        }
      
        // actually the camera that moves and not the model.
        dx -= vel_x * c_force_x * frame_delta_time;
        dy -= vel_y * c_force_y * frame_delta_time;
      
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

        // model, projection, view;
        // mvp, 

        float line_color[3] = { 1.0f, 1.0f, 1.0f };

        mat4 m_model, m_proj, m_view, m_mvp;

        identity_mat4(&m_model);
        identity_mat4(&m_proj);
        identity_mat4(&m_view);
        identity_mat4(&m_mvp);
        
        // perspective
        float fov = 90.0f;
        float aspect_ratio = 640.0f / 480.0f;
        float z_near = 0.001f;
        float z_far = 1000.0f;
        perspective_mat4(fov * A2R, aspect_ratio, z_near, z_far, &m_proj);

        // camera / lookat -> view
        vec3 eye, dir, up;
        // horrid
        // set_vec3(dx, dy, 1, &eye);

        set_vec3(0, 0, dy, &eye);
        set_vec3(0, 0, -1, &dir);
        set_vec3(0, 1, 0, &up); 
        lookat_mat4(eye, dir, up, &m_view);
        
        // mul opengl 
        mul_mat4(&m_proj, &m_view, &m_mvp); 

        glBindVertexArray(line_vao);
        glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
        glUseProgram(line_shader);

        glUniform3fv(line_shader_color_loc, 1, (GLfloat*)line_color);
        glUniformMatrix4fv(line_shader_mvp_loc, 1, GL_FALSE, (GLfloat*)m_mvp.v);

        glUniformMatrix4fv(proj_loc, 1, GL_FALSE, (GLfloat*)m_proj.v);
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, (GLfloat*)m_view.v);
        
        line_color[0] = line_color[1] = line_color[2] = 1.0f;
        glUniform3fv(line_shader_color_loc, 1, (GLfloat*)line_color);
        glDrawArrays(GL_TRIANGLES, 0, 3); 

        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);      
        #endif
        
        glFlush();
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
        float percent[TT_MAX];

        int i = 0;
        while(i < TT_MAX) {
            runtime += total_timing[i];
            i++;
        }

        active_frame_time = total_timing[TT_INPUT] + total_timing[TT_COMPUTE] + total_timing[TT_RENDER];

        printf("\nruntime info:\n");
        printf("num frames:    %llu\n", frame_count);
        printf("startup time:  %llu ms\n", total_timing[TT_INIT] / 1000);
        printf("cleanup time:  %llu ms\n", total_timing[TT_DEINIT] / 1000);

        // calc percent
        for(i = 0; i < TT_MAX; i++) {
            percent[i] = (float)total_timing[i] / (float)runtime; // 0..1
            percent[i] *= 100.0f; // to scale 0..100
        }   

        printf("\n");
        printf("frame_timings:\n");
        printf("input time:    %'9llu ms (%-5.2f %%)\n", total_timing[TT_INPUT] / 1000, percent[TT_INPUT]);
        printf("update time:   %'9llu ms (%-5.2f %%)\n", total_timing[TT_COMPUTE] / 1000, percent[TT_COMPUTE]);
        printf("render time:   %'9llu ms (%-5.2f %%)\n", total_timing[TT_RENDER] / 1000, percent[TT_RENDER]);
        printf("sleep time:    %'9llu ms (%-5.2f %%)\n", total_timing[TT_SLEEP] / 1000, percent[TT_SLEEP]);
        
        printf("\ntotal runtime: %'9llu ms\n", runtime / 1000); 
    }

    return 0;
}
