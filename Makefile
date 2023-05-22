CPP = g++
CFLAGS = -g -O2 -Wall -Wextra -pedantic -std=c++20
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lm

OUTNAME = main
INO = main.o pong/object.o utils.o scene.o ui.o pong/pong.o
INHPP = pong/object.hpp utils.hpp scene.hpp ui.hpp pong/pong.hpp

# TODO: Don't require a re-build of everything when you change a header

build: $(INO) $(INHPP)
		$(CPP) $(CFLAGS) -o $(OUTNAME) $(INO) $(LDFLAGS)

%.o: %.cpp
		$(CPP) -c ${CFLAGS} $< -o $@
