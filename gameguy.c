#include <SDL2/SDL.h>
#include <OpenGL/gl3.h>

#include <stdlib.h>
#include <stdbool.h>
// todo(stephen): Do reloading in a platform independent way.
#include <dlfcn.h>
// todo(stephen): Do random numbers in a platform independent way.
#include <time.h>

#include "gameguy.h"

#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg_gl.h"

// todo(stephen): Include in game config object.
// todo(stephen): Screen resizing does not work.


typedef struct gg_CurrentGame {
    void *handle;
    ino_t id;
    gg_Game game;
} gg_CurrentGame;


float
gg_get_seconds_elapsed(uint64_t old_counter, uint64_t current_counter)
{
    return ((float)(current_counter - old_counter) /
            (float)SDL_GetPerformanceFrequency());
}


// todo(stephen): Print out other non handled events.
// todo(stephen): Break out input handling into a seperate function so we can
//                use SDL's watch event stuff.
// todo(stephen): Have a way to record input over time, possibly using a
//                persistent data structure if that turns out to be helpful.
bool
gg_handle_event(SDL_Event *event, int *other_events_this_tick,
        gg_Input *input_state)
{
    bool should_quit = false;

    switch(event->type) {
        case SDL_QUIT:
            should_quit = true;
            break;

            // note(stephen): Going to use mouse events for now.
            // todo(stephen): use touch events when ready to port to ipad.
            // todo(stephen): figure out how to just do a click.
            // note(stephen): might want zooming
        case SDL_MOUSEBUTTONDOWN:
            /* log_info("mouse down"); */
            input_state->click = true;
            input_state->is_dragging = true;
            break;

        case SDL_MOUSEBUTTONUP:
            /* log_info("mouse up"); */
            input_state->is_dragging = false;
            input_state->end_dragging = true;
            break;

        case SDL_MOUSEMOTION:
            input_state->mouse_x = event->motion.x;
            input_state->mouse_y = event->motion.y;
            break;

            // todo(stephen): SDL_SetWindowFullscreen when hitting f.
        case SDL_KEYDOWN: {
            log_info("keypress");
            /* log_info("keypress"); */
            SDL_Keycode keycode = event->key.keysym.sym;

            if (SDLK_ESCAPE == keycode) {
                should_quit = true;
            }
        } break;

        default:
            // todo(stephen): print out that other events happened for debugging
            *other_events_this_tick+= 1;
            break;
    }

    return should_quit;
}

int main(int argc, char* argv[])
{
    // todo(stephen): Error if library not specified.
    char* game_library = "libgame.dylib";
    log_info("Loading Game: %s", game_library);

    // todo(stephen): function for reloading.
    gg_CurrentGame current_game;

    if (current_game.handle) {
        dlclose(current_game.handle);
    }

    current_game.handle = dlopen(game_library, RTLD_NOW);
    if (!current_game.handle) {
        log_info("Error loading library %s", dlerror());
    }

    gg_Game *game = dlsym(current_game.handle, "gg_game_api");
    current_game.game = *game;

    // todo(stephen): Figure out a better way to do random numbers.
    srand(time(NULL));

    // todo(stephen): Make this part of the config object.
    float game_update_hz = 60.0;
    float target_seconds_per_frame = 1.0f / game_update_hz;

    // todo(stephen): have a better error handling scheme that has all of these
    //                goto the end and write an error message instead of a bunch
    //                of different checks.
    if (0 != SDL_Init(SDL_INIT_VIDEO)) {
        log_info("Error initializing SDL: %s", SDL_GetError());
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
            SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // todo(stephen): Have window name come from config object.
    SDL_Window *window = SDL_CreateWindow("rockets",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            SDL_WINDOW_OPENGL |
                    SDL_WINDOW_RESIZABLE |
                    SDL_WINDOW_ALLOW_HIGHDPI);

    int w, h;
    SDL_GL_GetDrawableSize(window, &w, &h);
    log_info("Drawable Resolution: %d x %d", w, h);

    if (!window) {
        log_info("Error creating window: %s", SDL_GetError());
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);

    // Use Vsync
    if (SDL_GL_SetSwapInterval( 1 ) < 0) {
        log_info( "Warning: Unable to set VSync! SDL Error: %s", SDL_GetError());
    }

    // Setup nvg
    NVGcontext* vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
    if (vg == NULL) {
        log_error("Could not init nanovg.");
    }

    // Init Game
    void *gamestate;
    gamestate = current_game.game.init(vg);

    gg_Input input = {.mouse_x = 0,
            .mouse_y = 0,
            .click = false,
            .is_dragging = false,
            .end_dragging = false};

    bool running = true;
    uint64_t last_counter = SDL_GetPerformanceCounter();

    while (running) {
        // Reload Library
        // todo(stephen): only reload if file has changed.
        if (NULL != current_game.handle) {
            dlclose(current_game.handle);
        }

        current_game.handle = dlopen(game_library, RTLD_NOW);
        if (!current_game.handle) {
            log_info("Error loading library %s", dlerror());
        }

        gg_Game *game = dlsym(current_game.handle, "gg_game_api");
        current_game.game = *game;

        //todo(stephen): better event handling
        SDL_Event event;
        int other_events_this_tick = 0;

        input.end_dragging = false;;
        input.click = false;

        while (SDL_PollEvent(&event)) {
            if (gg_handle_event(&event,
                    &other_events_this_tick,
                    &input)) {
                running = false;
            }
        }
        if (other_events_this_tick > 0) {
            /* log_info("%d unhandled events this tick.",
               other_events_this_tick); */
        }

        /* Clear Screen */
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT |
                GL_STENCIL_BUFFER_BIT);

        // drawable resolution is
        /* 2872 x 1710 */
        // todo(stephen): Support non retina.
        nvgBeginFrame(vg, SCREEN_WIDTH, SCREEN_HEIGHT, 2.0);

        /* run game tick */
        current_game.game.update_and_render(gamestate, vg, input,
                                            target_seconds_per_frame);

        /* end frame */
        // todo(stephen): Can draw any admin console stuff here too like error
        // messages on lib reloads or fps or whatever.
        nvgEndFrame(vg);

        /* update the screen */
        // todo(stephen): I think this waits for vsync but maybe see if I should
        // also delay myself until vsync.
        SDL_GL_SwapWindow(window);

        uint64_t end_counter = SDL_GetPerformanceCounter();
        /* float full_frame_seconds_elapsed = */
        /*    sdl_get_seconds_elapsed(last_counter, end_counter); */
        /* log_info("%f", full_frame_seconds_elapsed); */

        last_counter = end_counter;

    }

    // Clean Up
    log_info("Cleaning up");
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}