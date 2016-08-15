#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include <new>

#include "module.h"
#include "console.h"
#include "taskqueue.h"

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

static const char _404_response_head[] = "\
HTTP/1.1 404 Not Found\r\n\
Content-Type: text/plain; charset=utf-8\r\n\
\r\n\
404 not found!\n\
Try following avaliable modules:\n\
";

static const char response_400[] = "\
HTTP/1.1 400 Bad Request\r\n\
Content-Type: text/html; charset=utf-8\r\n\
\r\n\
Bad Request.\n\
";

static const char root_response_head[] = "\
HTTP/1.1 200 OK\r\n\
Content-Type: text/plain; charset=utf-8\r\n\
\r\n\
Welcome!\n\
This server WORKS!!!\n\
Following modules are working:\n\
";

void sendstr(int connectFD, const char * response_body) {
	int count = strlen(response_body);
#define MAXSIZE 8192
	while (count > MAXSIZE) {
		if (send(connectFD, response_body, MAXSIZE, 0) == -1) {
			perror("send response");
			printf("fail send response");
			break;
		}
	}
#undef MAXSIZE
	send(connectFD, response_body, count, 0);
}

void connectionHandler(int connectFD) {
	printf("\n[connection]");

	if(connectFD < 0) {
		perror("accept connection");
		printf("fail accept connection");
		failExit(0);
	}

	// read request head and get path
	printf(" receiving...");
	char recvbuff[1024];
	recvbuff[0] = '\0';// prevent empty input
	char * path = recvbuff;
	for (;;) {// receiving
		int count = recv(connectFD, recvbuff, sizeof(recvbuff), 0);
		if (count > 0) {
			if (recvbuff[0] != 'G') { path[0] = '\0'; break; }
			if (recvbuff[1] != 'E') { path[0] = '\0'; break; }
			if (recvbuff[2] != 'T') { path[0] = '\0'; break; }
			path += 4;
			int i = 0;
			if (path[0] != '/') { path[0] = '\0'; break; }
			while(path[i] != ' ')
				if(i < sizeof(recvbuff) - 4) i++;
				else { i = 0; break; }// prevent overflow
			path[i] = '\0';
			break;
		}
		if (count == 0)
			break;
	}
	printf(" done.");

	// send response
	if (strcmp(path, "")) { // if path legal
		if (!strcmp(path, "/")) {
			printf(" responding root...");
			// send response
			sendstr(connectFD, root_response_head);
			sendstr(connectFD, mod_list);
			printf(" done.");
		} else {
			path++; // skip '/'
			// check query
			const char * params;
			for (int i = 0;; i++) {
				if(path[i] == '\0') { params = path + i; break;}
				if(path[i] == '?') { path[i] = '\0'; params = path + i + 1; break; }
			}
			printf(" try serving with %s...", path);
			char response_body[65536];
			if (mod_serve(path, params, response_body)) {
				// send head
				send(connectFD, response_200_head, sizeof(response_200_head) - 1, 0);
				// send body
				sendstr(connectFD, response_body);
				printf(" done.");
			} else {
				printf("\n[error] %s not found", path);
				printf(" responding...");
				// send head
				send(connectFD, _404_response_head, sizeof(_404_response_head) - 1, 0);
				// get/send body
				sendstr(connectFD, mod_list);
				printf("done.");
			}
		}
	} else { // if path illegal
		printf("\n[error] bad request");
		printf(" responeding...");
		send(connectFD, response_400, sizeof(response_400) - 1, 0);
		printf(" done.");
	}

	// close connection
	shutdown(connectFD, SHUT_RDWR);
	close(connectFD);
	printf("finished.\n");
	fflush(stdout);
}

void * connectionAccepter(void *) {
	TaskQueue<int> conqueue(connectionHandler);
	for (;;) {
		int connectFD = accept(socketFD, NULL, NULL);
		conqueue.addTask(connectFD);
	}
	return NULL;
}

int main(int argc, char ** argv) {
	int http_port = 8080;
	argv++; argc--;
	while (argc > 0) {
		if (!strcmp(argv[0], "--port")) {
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

	struct sigaction sa;
	sa.sa_handler = SIG_IGN;
	sigaction( SIGPIPE, &sa, 0 );
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

	// accept and handle connections
	pthread_t thread;
	if (pthread_create(&thread, NULL, connectionAccepter, NULL)) { fprintf(stderr, "fail create thread."); }

	console();

	while (close(socketFD) == -1);
	return 0;
}
