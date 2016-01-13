ifeq (${DEBUG},1)
CFLAGS = -O0 -ggdb -std=c++11
else
CFLAGS = -O2 -std=c++11
endif

INCL = ./include
LDFLAGS = -lpthread -L. -lsstm
SRCPATH = ./src

default: libsstm.a
	cc ${CFLAGS} -I${INCL} src/bank.c -o bank ${LDFLAGS}
	cc ${CFLAGS} -I${INCL} src/ll.c -o ll ${LDFLAGS}

clean:
	rm ./libsstm.a
	rm bank *.o src/*.o ll


$(SRCPATH)/%.o:: $(SRCPATH)/%.cpp include/sstm.h include/sstm_alloc.h
	g++ $(CFLAGS) -I ${INCL} -o $@ -c $<

.PHONY: libsstm.a

test: libsstm.a
	g++ ${CFLAGS} -I${INCL} src/test.cpp -o test ${LDFLAGS} -pthread

libsstm.a:	src/sstm.o src/sstm_alloc.o src/GlobalClock.o src/TSTMLock.o src/TSTMLockArray.o src/TSTMMemory.o
	ar cr libsstm.a src/sstm.o src/sstm_alloc.o src/GlobalClock.o src/TSTMLock.o src/TSTMLockArray.o src/TSTMMemory.o

