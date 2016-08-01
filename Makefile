CPP=g++
CPPFLAGS=-pthread -ldl -g
HEADS=module.h console.h
OBJECTS=server.o module.o console.o

bin/server: ${OBJECTS}
	g++ ${CPPFLAGS} ${OBJECTS} -o $@

%.o: %.cc
	g++ ${CPPFLAGS} -c $< -o $@

clean:
	rm -f ${OBJECTS} || true
