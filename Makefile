#build executable file
build:out

#make objs
obj/main.o: src/main.c
	gcc -o obj/main.o src/main.c -I include -c

out: obj/main.o
	gcc -o out obj/main.o

all: clean out

clean:
	$(RM) out obj/main.o