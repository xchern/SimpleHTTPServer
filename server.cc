#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <new>

#include "module.h"

int socketFD;

// SIGINT handler
void failExit(int signo) {
	fprintf(stderr, "Quitting\n");
	close(socketFD);
	exit(EXIT_FAILURE);
}

// http related
static const char response_200_head[] = "\
HTTP/1.1 200 OK\r\n\
Content-Type: text/html; charset=utf-8\r\n\
\r\n\
";

static const char response_404_head[] = "\
HTTP/1.1 404 Not Found\r\n\
Content-Type: text/html; charset=utf-8\r\n\
\r\n\
";

static const char response_400[] = "\
HTTP/1.1 400 Bad Request\r\n\
Content-Type: text/html; charset=utf-8\r\n\
\r\n\
Bad Request.\n\
";


static char root_response[] = "This server works";

void connectionHandler(int connectFD) {
	if(connectFD < 0) {
		perror("error accept failed");
		failExit(0);
	}

	fprintf(stderr, "NEW connection\n");

	// read request head and get path
	fprintf(stderr, "receiving...");
	char recvbuff[128];
	recvbuff[0] = '\0';// prevent empty input
	char * path = recvbuff;
	for (;;) {// receiving
		int count = recv(connectFD, recvbuff, sizeof(recvbuff) - 1, 0);
		if (count > 0) {
			if (recvbuff[0] != 'G') { path[0] = '\0'; break; }
			if (recvbuff[1] != 'E') { path[0] = '\0'; break; }
			if (recvbuff[2] != 'T') { path[0] = '\0'; break; }
			path += 4;
			int i = 0;
			if (path[0] != '/') { path[0] = '\0'; break; }
			while(path[i] != ' ')
				if(i < sizeof(recvbuff)) i++;
				else { i = 0; break; }// prevent overflow
			path[i] = '\0';
			break;
		}
		if (count == 0)
			break;
	}
	fprintf(stderr, "done\n");

	// send response
	if (strcmp(path, "")) { // if path legal
		if (!strcmp(path, "/")) {
			fprintf(stderr, "responding root...");
			// send head
			send(connectFD, response_200_head, sizeof(response_200_head), 0);
			// get body
			const char * response_body = root_response;
			// send body
			int count = strlen(response_body);
			send(connectFD, response_body, count, 0);
			fprintf(stderr, "done\n");
		} else {
			path++;
			int i = 0;
			while(path[i] != '\0') {
				if(path[i] == '?') break;
				i++;
			}
			path[i] = '\0';
			const char * params = path + i;
			if (modExistP(path)) {
				fprintf(stderr, "serving with %s...", path);
				// send head
				send(connectFD, response_200_head, sizeof(response_200_head), 0);
				// get body
				const char * response_body = modServe(path, params);
				// send body
				int count = strlen(response_body);
#define MAXSIZE 8192
				while (count > MAXSIZE) {
					send(connectFD, response_body, MAXSIZE, 0);
				}
#undef MAXSIZE
				send(connectFD, response_body, count, 0);
				fprintf(stderr, "done\n");
			} else {
				fprintf(stderr, "%s not found\n", path);
				fprintf(stderr, "responding...");
				// send head
				send(connectFD, response_404_head, sizeof(response_404_head), 0);
				//// get body
				//const char * response_body = root_response;
				//// send body
				//int count = strlen(response_body);
				//send(connectFD, response_body, count, 0);
				fprintf(stderr, "done\n");
			}
		}
	} else { // if path illegal
		fprintf(stderr, "bad request\n");
		fprintf(stderr, "responeding...");
		send(connectFD, response_400, sizeof(response_400), 0);
		fprintf(stderr, "done\n");
	}

	// close connection
	shutdown(connectFD, SHUT_RDWR);
	close(connectFD);
	fprintf(stderr, "finished.\n\n");
}


int main(int argc, char ** argv) {
	int http_port = 8080;
	argv++; argc--;
	while (argc > 0) {
		if (strcmp(argv[0], "-p")) {
			argv++; argc--;
			int port = atoi(argv[0]);
			if (port > 0)
			http_port = port;
		}
		argv++; argc--;
	}
	// new socket
	socketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(socketFD == -1) {
		perror("create socket");
		exit(EXIT_FAILURE);
	}
	fprintf(stderr, "Socket Created\n");

	// handle SIGINT
	if(signal(SIGINT, failExit) == SIG_ERR) {
		perror("handle signal");
		failExit(0);
	}
	fprintf(stderr, "Signal Interupt Handled\n");

	// bind socket on port 8080
	struct sockaddr_in stSockAddr;
	memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(http_port);
	stSockAddr.sin_addr.s_addr = INADDR_ANY;
	if(bind(socketFD, (const struct sockaddr *) &stSockAddr, sizeof(struct sockaddr_in)) == -1) {
		perror("bind port");
		failExit(0);
	}
	fprintf(stderr, "Port %d binded\n", http_port);

	if(listen(socketFD, 12) == -1) {
		perror("listen");
		failExit(0);
	}
	fprintf(stderr, "Start Listening\n\n");


	doLoad("modules/demo.so");

	// accept and handle connections
	for(;;) {
		int connectFD = accept(socketFD, NULL, NULL);
		connectionHandler(connectFD);
	}

	doUnload("demo");

	close(socketFD);
	return 0;
}
