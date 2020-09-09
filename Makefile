
CPP=g++
CC=gcc
CFLAGS=-I. -g -pthread
CPPFLAGS=-I. -g -pthread -DMY_CPP

DEPS = link.h defines.h pack.h gen.h
OBJ = main.o pack.o gen.o thread_base.o det.o log.o

%.o: %.c %.h
	$(CC) -g -Wall -o0 -c -o $@ $< $(CFLAGS)

%.o: %.cpp %.h $(DEPS)
	$(CPP) -g -Wall -o0 -c -o $@ $< $(CPPFLAGS)

app: $(OBJ)
	$(CPP) -o0 -o $@ $(OBJ) $(CPPFLAGS)


clean:
	rm -f *.o 
	rm app
	


