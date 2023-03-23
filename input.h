#ifndef STG_INPUT_H
#define STG_INPUT_H

#include <SDL2/sdl.h>

/*
    TODO:
        make input module actually usable
        add controller support
        add joystick support

        add polling for PRESSED / DOWN / VALUE
        add per-action registers for how it should ideally read input values
            (key instead of controller joystick axis)

        add recording / streaming io for testing (independant of FPS)
*/

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

#define NUM_ACTIONS     4
struct input {
    int is_remapping;
    int im_index;
    struct index_map c_im_kb;

    // current frame
    int                     in_kb[SDL_NUM_SCANCODES];
    struct index_map        im_kb[NUM_ACTIONS];
    struct input_action_key iak[NUM_ACTIONS];

    // prev frame
    int                     in_kb_prev[SDL_NUM_SCANCODES];
    struct index_map        im_kb_prev[NUM_ACTIONS];
    struct input_action_key iak_prev[NUM_ACTIONS];
};

void init_input(struct input * inp) {
    inp->is_remapping = 0;
    inp->im_index = 0;
    inp->c_im_kb.from = -1;
    inp->c_im_kb.to = -1;
    
    memset(inp->in_kb, 0, sizeof(int) * SDL_NUM_SCANCODES);
    memset(inp->in_kb_prev, 0, sizeof(int) * SDL_NUM_SCANCODES);
    
    inp->im_kb[0].from = SDL_SCANCODE_LEFT;
    inp->im_kb[0].to = 0;
    inp->im_kb[1].from = SDL_SCANCODE_RIGHT;
    inp->im_kb[1].to = 1;
    inp->im_kb[2].from = SDL_SCANCODE_DOWN;
    inp->im_kb[2].to = 2;
    inp->im_kb[3].from = SDL_SCANCODE_UP;
    inp->im_kb[3].to = 3;

    memset(inp->iak, 0, sizeof(struct input_action_key) * NUM_ACTIONS);
    memset(inp->iak_prev, 0, sizeof(struct input_action_key) * NUM_ACTIONS);
    inp->iak[0].tag = 0;
    inp->iak[1].tag = 1;
    inp->iak[2].tag = 2;
    inp->iak[3].tag = 3;
}

void do_input(struct input * inp) {
    struct index_map _im;
    for(int i = 0; i < NUM_ACTIONS; i++) {
        _im = im_kb[i];
        if(_im.from > -1 && _im.to > -1) {
            iak[_im.to].value.i = in_kb[_im.from];
        } else {
            // something is wrong with the mapping, clear to 0
            if(_im.to > -1) iak[_im.to].value.i = 0;
        }
    }
}

void do_input_remapping(struct input * inp) {
    // check if remapping is to be done
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

    // logic for the swaps
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
}

#endif /* STG_INPUT_H */
