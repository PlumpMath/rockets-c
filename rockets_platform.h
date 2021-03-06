#ifndef _rockets_platform_h
#define _rockets_platform_h

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <sys/stat.h>

// @TODO: Make this cross platform.
#include <OpenGL/gl3.h>

#include "nanovg.h"
// Libraries being used
// nanovg

#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720

// Library Functions
#define log_info(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

uint64_t performance_counter();   // Returns high frequency performance counter.
uint64_t performance_frequency(); // Returns number of performance counter ticks per second.

// Interface with Game.
typedef struct {
    int down_time; // ms, used internally.
    
    int mouse_x;
    int mouse_y;
    int mouse_xrel;
    int mouse_yrel;

    bool mouse_down;
    bool mouse_up;
    
    bool mouse_motion;
    bool click;
    bool holding;
    bool start_dragging;
    bool is_dragging;
    bool end_dragging;
} gg_Input;


typedef struct {
    int gamestate_size;
    void* (*init)(void* game_state, NVGcontext* vg);
    void (*update_and_render)(void* game_state,
                              NVGcontext* vg,
                              gg_Input input,
                              float dt);
} gg_Game;

extern const gg_Game gg_game_api;

#endif
