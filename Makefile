CPP=g++
CPPFLAGS=-pthread -ldl -g
HEADS=module.h console.h taskqueue.h queue.h
OBJECTS=server.o module.o console.o

bin/server: ${OBJECTS} bin/
	g++ ${CPPFLAGS} ${OBJECTS} -o $@

bin/:
	mkdir bin

%.o: %.cc ${HEADS}
	g++ ${CPPFLAGS} -c $< -o $@

clean:
	rm -f ${OBJECTS} || true
cpmod:
	cp modules/demo.so bin/modules/
