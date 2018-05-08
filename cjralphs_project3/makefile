all: project3

project3: project3.o node0.o node1.o node2.o node3.o
	gcc -g project3.o node0.c node1.c node2.c node3.c -o project3

project3.o: project3.c
	gcc -g -c project3.c -o project3.o

node0.o: node0.c
	gcc -g -c node0.c  -o node0.o

node1.o: node1.c
	gcc -g -c node1.c  -o node1.o

node2.o: node2.c
	gcc -g -c node2.c  -o node2.o

node3.o: node3.c
	gcc -g -c node3.c  -o node3.o

clean:
	rm -f *.o project3.o node0.o node1.o node2.o node3.o
