CFLAGS=`pkg-config --cflags sdl2` -std=c99 -g -Wall -O0
LDFLAGS=`pkg-config --libs sdl2`

main: sdl_platform.c game.h libgame.so
	clang $(CFLAGS) $(LDFLAGS) -o game sdl_platform.c

libgame.so: game.c game.h
	clang $(CFLAGS) -shared $(LDFLAGS) -o libgame.so

clean:
	rm -rf *o *.dSYM game
