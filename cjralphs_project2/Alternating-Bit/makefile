all: project2

project2: project2.o project2.h student2.o
	gcc -g project2.o student2.o -o project2

project2.o: project2.c
	gcc -g -c project2.c  -o project2.o

student2.o: student2.c
	gcc -g -c student2.c  -o student2.o

clean:
	rm -f *.o project2.o student2.o
