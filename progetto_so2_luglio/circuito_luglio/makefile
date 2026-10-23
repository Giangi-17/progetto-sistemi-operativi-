CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lm -lpthread

circuito: main.o complex.o matrix.o vector.o multithread.o parser.o misurazione.o
	$(CC) $(CFLAGS) -o circuito main.o complex.o matrix.o vector.o multithread.o parser.o misurazione.o $(LDFLAGS)

main.o: main.c parser.h vector.h matrix.h multithread.h misurazione.h
	$(CC) $(CFLAGS) -c main.c

complex.o: complex.c complex.h
	$(CC) $(CFLAGS) -c complex.c

matrix.o: matrix.c vector.h complex.h matrix.h
	$(CC) $(CFLAGS) -c matrix.c

vector.o: vector.c vector.h complex.h
	$(CC) $(CFLAGS) -c vector.c

multithread.o: multithread.c multithread.h matrix.h vector.h
	$(CC) $(CFLAGS) -c multithread.c

parser.o: parser.c parser.h complex.h vector.h matrix.h
	$(CC) $(CFLAGS) -c parser.c

misurazione.o: misurazione.c misurazione.h complex.h vector.h
	$(CC) $(CFLAGS) -c misurazione.c

clean:
	rm -f *.o circuito