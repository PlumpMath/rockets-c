// I would really like to write some sofisticated algorithms for something some day.
// precedural generation
// AI
// mcts monte carlo tree search rockets solver maybe or something like that?

// For the actual gameplay how do I support
// * Math 
// * Lists / stacks / variable amounts of things
// * Recursion? For loops? Map/fold/filter?

// For math I can have two types of nodes, int nodes and bool nodes. That's ok I guess.
// I could say no variable length stuff and just have different signals for each level.
// How impossible is it going to be to do your own vector math in the nodes.
#include "gameguy.h"
#include "game.h"

static NVGcontext* gg_Debug_vg = NULL;

// Rendering Code
void
draw_parent_line(NVGcontext* vg, const Node* node, const Node* parent)
{
    if (parent) {
        nvgSave(vg);
        {
            nvgStrokeColor(vg, nvgRGBf(200.0, 200.0, 200.0));
            nvgBeginPath(vg);
            nvgTextLineHeight(vg, 100);
            nvgMoveTo(vg, node->position.x, node->position.y);
            nvgLineTo(vg, parent->position.x, parent->position.y);
            nvgStroke(vg);
        }
        nvgRestore(vg);
    }
}

void
ship_move(Ship* ship, float dt)
{
    V2 force = v2(0.0, 0.0);
    int rotation = 0;

    if (ship->thrusters.bp) {
        force = v2_plus(force, v2(1, 0));
        rotation -= 1;
    }

    if (ship->thrusters.bs) {
        force = v2_plus(force, v2(-1, 0));
        rotation += 1;
    }

    if (ship->thrusters.sp) {
        force = v2_plus(force, v2(1, 0));
        rotation += 1;
    }

    if (ship->thrusters.ss) {
        force = v2_plus(force, v2(-1, 0));
        rotation -= 1;
    }

    if (ship->thrusters.boost) {
        force = v2_plus(force, v2(0, 5));
    }

    V2 abs_force = v2_rotate(force, deg_to_rad(ship->rotation));
    float speed = 50;

    ship->position.x += abs_force.x*speed*dt;
    ship->position.y += abs_force.y*speed*dt;
    int new_rot = ship->rotation + rotation;
    if (new_rot < 0) new_rot += 360;
    ship->rotation = new_rot % 360;
}


int
ship_get_signal(const Ship* ship, Signal signal)
{
    switch(signal) {
    case POS_X:
        return ship->position.x;
        break;
    case POS_Y:
        return ship->position.y;
        break;
    case ROTATION:
        return ship->rotation;
        break;
    }

    //error
    return -1;
}


int
node_eval_sub_node(const Node* node, const Ship* ship)
{
    if (node->type == SIGNAL) {
        return ship_get_signal(ship, node->signal);
    } else { // Constant
        return node->constant;
    }
}


bool
node_eval(const Node* node, const NodeStore* ns, const Ship* ship)
{
    if (NULL == node) {
        return false;
    }

    // SOME DEBUG PRINTING!
    switch(node->type) {
    case SIGNAL: {
        // only a sub node
    } break;
    case CONSTANT: {
        // only a sub node
    } break;
    case PREDICATE: {
        int lhs = node_eval_sub_node(nodestore_get_node_by_id(ns,
                                                              node->input.lhs),
                                     ship);
        int rhs = node_eval_sub_node(nodestore_get_node_by_id(ns,
                                                              node->input.rhs),
                                     ship);

        switch(node->predicate) {
        case LT:
            return lhs < rhs;
            break;
        case GT:
            return lhs > rhs;
            break;
        case LEQT:
            return lhs <= rhs;
            break;
        case GEQT:
            return lhs >= rhs;
            break;
        case EQ:
            return lhs == rhs;
            break;
        case NEQ:
            return lhs != rhs;
            break;
        }
    } break;
    case GATE: {
        bool lhs = node_eval(nodestore_get_node_by_id(ns, node->input.lhs),
                             ns, ship);
        bool rhs = false;

        if (node->gate != NOT) {
            rhs = node_eval(nodestore_get_node_by_id(ns, node->input.rhs),
                            ns, ship);
        }

        switch(node->gate) {
        case AND:
            return lhs && rhs;
            break;
        case OR:
            return lhs || rhs;
            break;
        case NOT:
            return !lhs;
            break;
        }

    } break;
    case THRUSTER: {
        return node_eval(nodestore_get_node_by_id(ns, node->parent), ns, ship);
    } break;
    }

    // This is actually an error tho...
    return false;

}


// @TODO: see comment in game.h on the nodestore on how to improve this.
Thrusters
nodestore_eval_thrusters(const NodeStore* ns, const Ship* ship)
{
    Thrusters out_thrusters = {false, false, false, false, false};

    // iterate over thruster nodes
    for (int i = 0; i < ns->next_id; i++) {
        Node* node = nodestore_get_node_by_id(ns, i);
        bool value = node_eval(node, ns, ship);

        // @TODO: IF DEBUG
        nvgSave(gg_Debug_vg);
        {
            if (value) {
                nvgFillColor(gg_Debug_vg, nvgRGBf(0,1,0));
            } else {
                nvgFillColor(gg_Debug_vg, nvgRGBf(1,0,0));
            }
            debug_draw_square(gg_Debug_vg, node->position.x-25, node->position.y);
        }
        nvgRestore(gg_Debug_vg);

        if (node->type == THRUSTER) {
            switch(node->thruster) {
            case BP:
                out_thrusters.bp |= value;
                break;
            case BS:
                out_thrusters.bs |= value;
                break;
            case SP:
                out_thrusters.sp |= value;
                break;
            case SS:
                out_thrusters.ss |= value;
                break;
            case BOOST:
                out_thrusters.boost |= value;
                break;
            }
        }
    }

    return out_thrusters;
}

// Really need to get farther in handmade hero and see how casey does stuff.
// Entity stores in tables, sparse storage, stuff like that.
void
gamestate_load_level_one(GameState* state) {
    state->player_ship.position.x = 300;
    state->player_ship.position.y = 99;
    state->player_ship.rotation = 0;
    state->num_obstacles = 0;

    state->goal = v2(300, 600);
    state->current_level = 1;
}

void
gamestate_load_level_two(GameState* state) {
    state->player_ship.position.x = 300;
    state->player_ship.position.y = 99;
    state->player_ship.rotation = 0;
    state->num_obstacles = 0;

    state->goal = v2(500, 600);
    state->current_level = 2;
}

void
gamestate_load_level_three(GameState* state) {
    state->player_ship.position.x = 300;
    state->player_ship.position.y = 99;
    state->player_ship.rotation = 0;

    state->obstacles[0] = boundingBox(v2(295, 595), 10, 10);
    state->obstacles[1] = boundingBox(v2(495, 95), 10, 10);
    state->num_obstacles = 2;

    state->goal = v2(500, 600);
    state->current_level = 3;
}


static void*
game_setup(NVGcontext* vg)
{
    log_info("Setting up game");
    GameState* state = calloc(1, sizeof(GameState));

    // @TODO: If you have more than one font you need to store a
    // reference to this.
    int font = nvgCreateFont(vg,
                             "basic",
                             "SourceSansPro-Regular.ttf");
    // Assert that the font got loaded.
    assert(font >= 0);

    nodestore_init(&state->node_store, 5);

    gamestate_load_level_three(state);

    return state;
}


static void
game_update_and_render(void* gamestate,
                       NVGcontext* vg,
                       gg_Input input,
                       float dt)
{
    // Setup debug frame.
    if (!gg_Debug_vg) {
        gg_Debug_vg = vg;
    }

    // Setup frame.
    GameState* state = (GameState*)gamestate;
    state->gui.vg = vg;
    state->gui.input = input;

    // Update Gui
    if (gui_button(state->gui, 10, 10, 50, 25)) {
        int a = nodestore_add_constant(&state->node_store, 0, 0, 0);
        int b = nodestore_add_signal(&state->node_store, 0, 0, POS_X);
        int c = nodestore_add_predicate(&state->node_store, 10, 45, EQ);
        Node* node_c = nodestore_get_node_by_id(&state->node_store, c);
        node_c->input.rhs = a;
        node_c->input.lhs = b;

        log_info("adding predicate node");
    }

    if (gui_button(state->gui, 70, 10, 50, 25)) {
        nodestore_add_gate(&state->node_store, 10, 45, AND);
        log_info("adding gate node");
    }

    if (gui_button(state->gui, 130, 10, 50, 25)) {
        nodestore_add_thruster(&state->node_store, 10, 45, BOOST);
        log_info("adding thruster node");
    }

    // @TODO: It is very unfortunate that rendering happens in here....
    NodeEvent event = gui_nodes(&state->gui, &state->node_store);
    switch(event.type) {
    case NE_NAH:
        break;
    }

    // @TODO: decide if you want a pause.
    /* state->running = true; */

    // Reset button
    // @TODO: reset the whole level not just the ship.
    if (gui_button_with_text(state->gui, 660, 2.5, 10, 5, "Reset")) {
            state->player_ship.position.x = 300;
            state->player_ship.position.y = 99;
            state->player_ship.rotation = 0;
            state->status = RUNNING;
    }

    // Update rockets.
    // @TODO: Collision detection!
    // Need to see if a rocket hits anything. I can do a minkowsky thing I think
    if (state->status == RUNNING) {
        Thrusters new_thrusters = nodestore_eval_thrusters(&state->node_store,
                                                           &state->player_ship);
        state->player_ship.thrusters = new_thrusters;
        ship_move(&state->player_ship, dt);
    }

    // Collision Detection.
    // Can't use bounding boxes for this as the rocket is not always axis aligned.
    // @HARDCODE
    // what do we do, like top and bottom and leftmost and rightmost?
    
    // If rocket hits something, you explode
    // If rocket goes off the screen, you are lost in space!

    // Evaluate anything that happened.
    // If rocket is in the goal circle, then you win!
    // TODO: use the whole rocket bb instead of just the center point.
    if (bounds_contains(state->goal.x - 10,
                        state->goal.y - 10,
                        state->goal.x + 10,
                        state->goal.y + 10,
                        state->player_ship.position.x, state->player_ship.position.y)) {

        // Won the level!
        state->status = WON;
        
    }

    // Render

    // Space background!
    nvgBeginPath(vg);
    nvgRect(vg, 660, 10, 600, 700);
    nvgFillColor(vg, nvgRGBf(0.25, 0.25, 0.25));
    nvgFill(vg);

    // Level info
    char buf[64] = {'\0'};
    snprintf(buf, 64, "level %d", state->current_level);
    debug_draw_text(vg, 660, 27, 24, buf);

    nvgSave(vg);
    {
        // x,y positions will need their y flipped to be drawn in the proper
        // place. The translates take care of the rest of the movement. Math in
        // space can be done in cartesian coordinates, just need this work to
        // draw in the right place. Drawing will be done in normal nvg
        // coordinates so stuff like text works. Collision detection will have
        // to be in cartesian and seperate from rendering.
        nvgTranslate(vg, 660, 10);
        nvgTranslate(vg, 0, 700);

        // @TODO: Figure out what you have to do to not have to make all y coordinates
        // negative here.

        // draw ship
        nvgSave(vg);
        {
            nvgTranslate(vg,
                         state->player_ship.position.x,
                         -state->player_ship.position.y);
            nvgRotate(vg, -deg_to_rad(state->player_ship.rotation));
            draw_ship(vg,
                      state->player_ship.thrusters,
                      false);
        }
        nvgRestore(vg);

        // draw obsticles
        for (int i=0; i < state->num_obstacles; i++) {
            nvgBeginPath(vg);
            nvgRect(vg,
                    state->obstacles[i].top_left.x,
                    -state->obstacles[i].top_left.y,
                    state->obstacles[i].bottom_right.x-state->obstacles[i].top_left.x,
                    -state->obstacles[i].bottom_right.y+state->obstacles[i].top_left.y);
            nvgFillColor(vg, nvgRGBf(0.0, 1.0, 1.0));
            nvgFill(vg);
        }

        // draw the goal
        nvgBeginPath(vg);
        nvgRect(vg, state->goal.x-10, -state->goal.y-10, 20, 20);
        nvgFillColor(vg, nvgRGBf(1.0,1.0,0));
        nvgFill(vg);
    }
    nvgRestore(vg);

    // @TODO: pull this out, this is hella useful.
    // debug print
    char buf2[64] = {'\0'};

    snprintf(buf2, 64, "position: (%f, %f), rotation: %d",
             state->player_ship.position.x,
             state->player_ship.position.y,
             state->player_ship.rotation);

    debug_draw_text(vg, 10, SCREEN_HEIGHT - 50, 24, buf2);

}


const gg_Game gg_game_api = {
        .init = game_setup,
        .update_and_render = game_update_and_render
};


const char*
bool_string(bool b)
{
    return b ? "true" : "false";
}
