#build executable file
build:out

#make objs
obj/lunch.o: src/lunch.c
	gcc -o obj/lunch.o src/lunch.c -I include -I library/linear_list -c

obj/main.o: src/main.c
	gcc -o obj/main.o src/main.c -I include -I library/linear_list -c

obj/linear_list.o: library/linear_list/linear_list.c
	gcc -o obj/linear_list.o library/linear_list/linear_list.c -I include -I library/linear_list -c

out: obj/lunch.o obj/main.o obj/linear_list.o
	gcc -o out obj/lunch.o obj/main.o obj/linear_list.o -lreadline

all: clean out

clean:
	$(RM) out obj/lunch.o obj/main.o obj/linear_list.o