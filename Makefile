#build executable file
build:out

#make objs
obj/lunch.o: src/lunch.c
	gcc -o obj/lunch.o src/lunch.c -I include -I library/linear_list -I library/nodeSystem -D NODE_SYSTEM_HOST -c

obj/main.o: src/main.c
	gcc -o obj/main.o src/main.c -I include -I library/linear_list -I library/nodeSystem -D NODE_SYSTEM_HOST -c

obj/linear_list.o: library/linear_list/linear_list.c
	gcc -o obj/linear_list.o library/linear_list/linear_list.c -I include -I library/linear_list -I library/nodeSystem -D NODE_SYSTEM_HOST -c

obj/nodeSystem.o: library/nodeSystem/nodeSystem.c
	gcc -o obj/nodeSystem.o library/nodeSystem/nodeSystem.c -I include -I library/linear_list -I library/nodeSystem -D NODE_SYSTEM_HOST -c

out: obj/lunch.o obj/main.o obj/linear_list.o obj/nodeSystem.o
	gcc -o out obj/lunch.o obj/main.o obj/linear_list.o obj/nodeSystem.o -lreadline

all: clean out

clean:
	$(RM) out obj/lunch.o obj/main.o obj/linear_list.o obj/nodeSystem.o