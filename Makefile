RM = rm

#build executable file
build:out

#make objs
obj/main.o: src/main.c
	gcc -o obj/main.o src/main.c  -Iinclude -I/usr/include/freetype2 -c

obj/window-tree.o: src/window-tree.c
	gcc -o obj/window-tree.o src/window-tree.c  -Iinclude -I/usr/include/freetype2 -c

obj/linear_list.o: src/linear_list.c
	gcc -o obj/linear_list.o src/linear_list.c  -Iinclude -I/usr/include/freetype2 -c

out: obj/main.o obj/window-tree.o obj/linear_list.o 
	gcc -o out obj/main.o obj/window-tree.o obj/linear_list.o -lglut -lGLU -lGL -lm `pkg-config --cflags --libs ftgl`

all: clean out

clean:
	$(RM) out obj/main.o obj/window-tree.o obj/linear_list.o