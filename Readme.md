# Rockets
the c version, currently only works on osx

## Install Dependencies
* `brew install sdl2`
* `brew install premake` (A dependency of a dependency...)
* `git submodule init`
* `git submodule update --recursive`

* `cd nanovg`
* `premake4 gmake`
* `cd build`
* `make nanovg`
* `cd ../..`

## Make and run rockets
`make`

## Run the game
`./rockets`

## Recompile any changes to game.c and reload while running.
`make`

# changelog
* [before now]    Did a ton of work. Starting this changelog pretty deep into the process of developing this. I've built out a prototype of just about everything.

* [8/21/15]    Getting back into development here. I learned a lot of c since I last worked on this so the first thing I'm doing is dropping the stb stretchy buffer stuff. I decided I don't need it and I'm going to get rid of dynamic memory allocation for this. I want to redo the node stuff to also use up front static allocation and a free list. Gonna do that next (maybe today). The biggest things I need to tackle with this is how the UI works. It was all written by me very adhocily and it's not a very good user experience. I had nate try it out a few weeks ago and he had no idea what any of the buttons did (which makes sense because they are all just blue). Dragging connections between nodes is very non obvious so that's something I need to make nicer. I think I just need more hints on things when you scroll over them, etc. Gonna maybe just do a new version from scratch once I modify the node api anyway. I need there to be math in the nodes I think so you can calculate things. It's hard to say really what all should be available in the nodes. I wonder if there's some logic system that embodies all the actions one would want to take that I can map easily to a set of node rules.

* [8/21/15]    Did a pretty big rewrite of the nodestore. It now uses a fixed amount of memory and a freelist to manage instead of using malloc and free. That along with removing stb stretchy buffers means this is completely manual memory management again, only one allocation at startup, casey style. I like this a lot now that I know how to use it. I also added an actual hash map for indexing by id. I also got how to implement this from watching casey. I will probably also add a topological sort of the nodes at some point so that evaluation can be faster. Next I think I should spend some time on level designing and collision/entity stuff so that I can play with some more complicated levels and decide how I want the node stuff to actually work.

* [8/24/15]    I broke out the predicate nodes into actually having the signals and constants to be their own nodes and updated the GUI code to handle it. This is mostly to enable some future math nodes. I still have a lot of work to do on the GUI to make it nicer to use. I'm going to add some sliders and things for modifying values and show visual indicators when dragging inputs and outputs to show where you can drag them to.

* [8/26/15]    I did a first pass at a slider for the constant nodes. It seems to work but it has a lot of problems. 1 The code is pretty fragile. I'm having a lot of trouble seeing how I can actually take this UI code and turn it into something people are actually going to want to use. More and more I think I should use a real GUI toolkit like the browser to do all of this. The other big problem with this slider though is I can't hit every number on the spectrom. Unless the slider were 700 pixels long I can't actually toggle it to every number between 1 and 700. That's a problem because you probably have a specific value in mind you are trying to hit. I also don't want to just have the slider go 0 - 700 because for some things, like rotations that doesn't even make sense. The value you are toggeling should really depend on the nodes that depend on this constant node. UI stuff is very frustrating, this thing would be so easy in mathematica's UI toolkit, really wishing that was a library for c.

* [8/30/15]    Slider not being wide enough is sort of a deal breaker so I'm moving to buttons. I'm going to have an up and down button, and you can click them to change the value by 1 or hold them to change the value faster. The longer you hold it the faster the value changes so you can get around quickly then click it to hone in on what you want. This is a pretty common UI thing, roughly based on how this works. https://jqueryui.com/spinner/ I've also spent a lot of time thinking about how INGUI style ui could be done in larger apps. Most imgui libraries are just for debug or developer tools, not for player facing UI but I don't want to move to a "REAL" ui library so I have some ideas for ways to do widgets. I want to not have callbacks so I will have a little queue of events the player can loop through for stuff that happened that frame, I also want to have more sytling power, maybe build a flexbox layout model too. (I think that is what facebook is doing for react native and nikki was showing me it and it looked to be simple enough to do from scratch) This is all getting ahead of myself a bit, need to finish rockets and then will probably do a lot of this AI work on my AI game playing tools if I ever get to those.

[9/01/15]      Today was mostly procrastination. I got jelous of how good sublime is for golang and wanted to use it for c. I spent a long time figuring out how to do that and finding the sublime-clang plugin that actually makes this a pretty great IDE. One problem though was that it only seems to support c++, not c and I already kind of wanted operator and method overloading so I just switched the project over to c++. It works the same as it did before. I threw in some changes to my vector code that are enabled by the switch to c++ but it isn't really that much of a change.

[9/01/15]      That was a bad move. I broke my debugger and sublime isn't even that good and emacs is fine and I just reverted everything and am back in C.

[9/01/15]      Wrote an entity system and ported all the adhoc goal and ship handling code to use it. This is going to let me treat all the things in the scene in common ways and enable me to write some good collision detection code. This is similar to the way casey does stuff except a lot simpler because I don't have a high and low frequency update set or sim regions or any of that stuff. Everything in the scene is an entity. I'm going to now set velocity on the entities and write a general collision detection routine to handle moving them around. This way I can have planets move in the scene too. I'm going to need to do some work on fixing my timestep because I want this to all be deterministic, hitting replay with the same nodes should act the same way regardless of the framerate. Feeling really good about this!

[9/03/15]      Today I finially got around to writing collision detection. This was pretty fun. I used minkownski sums. I did a pretty simple version only dealing with AABB boxes. I think this is a good start. Next I need to allow for rotations and then also add in other possible shapes. Debugging this was pretty hard because I can't draw to the space scene from update code because the transform is wrong. I should either do all my space scene updates from within the right nvg transform or write some helper functions for this. I ended up fighing a lot of suble bugs in my 4 deep nested for loop. Things that even just having a foreach would have avoided. C is sort of tedious in that regard. All in all it went really well though and now I can design some levels. I want to have finding the goal, not hitting walls and running into objects in the scene all use the collision system. I didn't have to do much math about collision handling because if you hit something you either win or you die, I only had to write collision detection code. I will do a bit of collision handling but only to react to things like winning or losing, I wont be doing any "find where the player should be placed" math for this, I'll save that for the next game.