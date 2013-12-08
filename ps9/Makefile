test: sched.o savectx.o test.o
	gcc -m32 -o test sched.o savectx.o test.o

sched.o: sched.c sched.h
	gcc -c -g -m32 sched.c

savectx.o: savectx.c savectx.h jmpbuf-offsets.h
	gcc -c -g -m32 savectx.o

test.o: test.c
	gcc -c -g -m32 test.c

clean:
	rm -f *.out *.o *.stackdump *~