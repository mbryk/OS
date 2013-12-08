test: sched.o savectx64.o test.o
	gcc -o test sched.o savectx64.o test.o -lm

sched.o: sched.c sched.h
	gcc -c -g sched.c

savectx64.o: savectx64.S savectx64.h jmpbuf-offsets64.h
	gcc -c -g savectx64.S

test.o: test1.c
	gcc -c -g test1.c -o test.o

clean:
	rm -f *.out *.o *.stackdump *~