bin/server: server.o module.o
	g++ server.o module.o -ldl -o bin/server

server.o: server.cc
	g++ server.cc -c -o server.o

module.o: module.cc
	g++ module.cc -c -o module.o
