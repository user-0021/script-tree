#build executable file
build:out

#make objs
obj/main.o: src/main.c
	gcc -o obj/main.o src/main.c -I include -I /usr/include/freetype2 -I library/linear_list -I library/window-tree -c

obj/linear_list.o: library/linear_list/linear_list.c
	gcc -o obj/linear_list.o library/linear_list/linear_list.c -I include -I /usr/include/freetype2 -I library/linear_list -I library/window-tree -c

obj/window-tree.o: library/window-tree/window-tree.c
	gcc -o obj/window-tree.o library/window-tree/window-tree.c -I include -I /usr/include/freetype2 -I library/linear_list -I library/window-tree -c

out: obj/main.o obj/linear_list.o obj/window-tree.o
	gcc -o out obj/main.o obj/linear_list.o obj/window-tree.o -lglfw -lGL -lm `pkg-config --cflags --libs ftgl`

all: clean out

clean:
	$(RM) out obj/main.o obj/linear_list.o obj/window-tree.o