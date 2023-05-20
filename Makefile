CPP = g++
CFLAGS = -g -O2 -Wall -Wextra -pedantic -std=c++20
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lm

OUTNAME = main
INCPP = main.cpp pong/object.cpp utils.cpp scene.cpp ui.cpp pong/pong.cpp
INHPP = pong/object.hpp utils.hpp scene.hpp ui.hpp pong/pong.hpp

build: $(INCPP) $(INHPP)
		$(CPP) $(CFLAGS) -o $(OUTNAME) $(INCPP) $(LDFLAGS)
