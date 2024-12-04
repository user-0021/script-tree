#build executable file
build:out

#make objs
obj/terminal_splitter.o: src/terminal_splitter.c
	gcc -o obj/terminal_splitter.o src/terminal_splitter.c -I include -I library/linear_list -c

obj/main.o: src/main.c
	gcc -o obj/main.o src/main.c -I include -I library/linear_list -c

obj/lunch.o: src/lunch.c
	gcc -o obj/lunch.o src/lunch.c -I include -I library/linear_list -c

obj/linear_list.o: library/linear_list/linear_list.c
	gcc -o obj/linear_list.o library/linear_list/linear_list.c -I include -I library/linear_list -c

out: obj/terminal_splitter.o obj/main.o obj/lunch.o obj/linear_list.o
	gcc -o out obj/terminal_splitter.o obj/main.o obj/lunch.o obj/linear_list.o -lreadline

all: clean out

clean:
	$(RM) out obj/terminal_splitter.o obj/main.o obj/lunch.o obj/linear_list.o