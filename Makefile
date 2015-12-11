FILENAME=asteroids
SRCNAME=asteroids.c
STD=gnu11
MINGWSDLARGS=-L/usr/i686-w64-mingw32/lib -lmingw32 -lSDL2main -lSDL2 -mwindows -Wl,--no-undefined -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -static-libgcc
MINGWSDLIMAGEARGS=-L/usr/i686-w64-mingw32/lib -lSDL2_image -lmingw32 -lSDL2main -lSDL2 -mwindows -lmingw32 -lSDL2main -lSDL2 -mwindows -Wl,--no-undefined -lm -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid -static-libgcc -lpng16

all: asteroids

release: win32 *.h *.c
	clang $(SRCNAME) -o$(FILENAME) -std=$(STD) -lSDL2 -lSDL2_image -lm -lz -Wall -O2

$(FILENAME): *.h *.c
	clang $(SRCNAME) -o$(FILENAME) -std=$(STD) -lSDL2 -lSDL2_image -lm -lz -Wall -O0 -g

run: $(FILENAME)
	./$(FILENAME)

win32:
	i686-w64-mingw32-gcc $(SRCNAME) -o$(FILENAME).exe -std=$(STD) $(MINGWSDLARGS) $(MINGWSDLIMAGEARGS) -lSDL2_image -lpng16 -lm -lz -Wall -O2 -static

.PHONY: all run
