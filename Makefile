all:
	gcc -I src/include -L src/lib -o main *.c -lmingw32 -lSDL2main -lSDL2 -lSDL2_gfx -lSDL2_ttf -lSDL2_image -lSDL2_mixer -Werror -Wall