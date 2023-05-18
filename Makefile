CPP = g++
CFLAGS = -O2 -Wall -Wextra -pedantic -std=c++20
LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lm

OUTNAME = main
INCPP = main.cpp object.cpp utils.cpp 
INHPP = object.hpp utils.hpp

build: $(INCPP) $(INHPP)
		$(CPP) $(CFLAGS) -o $(OUTNAME) $(INCPP) $(LDFLAGS)
