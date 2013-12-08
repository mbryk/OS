test: sched.o savectx64.o test.o
	gcc -o test sched.o savectx64.o test.o

sched.o: sched.c sched.h
	gcc -c -g sched.c

savectx64.o: savectx64.c savectx64.h jmpbuf-offsets64.h
	gcc -c -g savectx64.c

test.o: test.c
	gcc -c -g test.c

clean:
	rm -f *.out *.o *.stackdump *~