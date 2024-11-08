#build executable file
build:out

#make objs
obj/main.o: src/main.c
	gcc -o obj/main.o src/main.c -I include -I /usr/include/freetype2 -I library/linear_list -I library/window-tree -I library/glad -c

obj/linear_list.o: library/linear_list/linear_list.c
	gcc -o obj/linear_list.o library/linear_list/linear_list.c -I include -I /usr/include/freetype2 -I library/linear_list -I library/window-tree -I library/glad -c

obj/window-tree.o: library/window-tree/window-tree.c
	gcc -o obj/window-tree.o library/window-tree/window-tree.c -I include -I /usr/include/freetype2 -I library/linear_list -I library/window-tree -I library/glad -c

obj/glad.o: library/glad/glad.c
	gcc -o obj/glad.o library/glad/glad.c -I include -I /usr/include/freetype2 -I library/linear_list -I library/window-tree -I library/glad -c

out: obj/main.o obj/linear_list.o obj/window-tree.o obj/glad.o
	gcc -o out obj/main.o obj/linear_list.o obj/window-tree.o obj/glad.o -lglfw -ldl -lGL -lm `pkg-config --cflags --libs ftgl`

all: clean out

clean:
	$(RM) out obj/main.o obj/linear_list.o obj/window-tree.o obj/glad.o