#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <new>

static const char response_head[] = "\
HTTP/1.1 200 OK\r\n\
Content-Type: text/html; charset=utf-8\r\n\
\r\n\
";

static char root_response[] = "";

static const char body404[] = "This server supports NO such funtion!\n";

int socketFD;

void failExit(int signo) {
	close(socketFD);
	exit(EXIT_FAILURE);
}

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
	recvbuff[sizeof(recvbuff) - 1] = ' ';// prevent overflow
	char * path = recvbuff;
	for (;;) {
		int count = recv(connectFD, recvbuff, sizeof(recvbuff) - 1, 0);
		if (count > 0) {
			if (path[0] != 'G') { path[0] = '\0'; break; }
			if (path[1] != 'E') { path[0] = '\0'; break; }
			if (path[2] != 'T') { path[0] = '\0'; break; }
			path += 4;
			int i = 0;
			while(path[i] != ' ') i++;
			path[i] = '\0';
			break;
		}
		if (count == 0)
			break;
	}
	fprintf(stderr, "done\n");

	// send response
	if (strcmp(path, "")) {
		fprintf(stderr, "sending...");
		send(connectFD, response_head, sizeof(response_head), 0);
		const char * response_body = NULL;
		if (!strcmp(path, "/")) {
			response_body = root_response;
		}
		if (!response_body) response_body = body404;
		int count = strlen(response_body);
		send(connectFD, response_body, count, 0);
		fprintf(stderr, "done\n");
	}
	shutdown(connectFD, SHUT_RDWR);
	close(connectFD);
	fprintf(stderr, "finished.\n\n");
}
int main(void) {
	// new socket
	socketFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(socketFD == -1) {
		perror("error create socket failed");
		exit(EXIT_FAILURE);
	}

	// handle SIGINT
	if(signal(SIGINT, failExit) == SIG_ERR) {
		perror("error handle signal failed");
		failExit(0);
	}

	// bind socket on port 8080
	struct sockaddr_in stSockAddr;
	memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(8080);
	stSockAddr.sin_addr.s_addr = INADDR_ANY;
	if(bind(socketFD, (const struct sockaddr *) &stSockAddr, sizeof(struct sockaddr_in)) == -1) {
		perror("error bind failed");
		close(socketFD);
		exit(EXIT_FAILURE);
	}
	if(listen(socketFD, 12) == -1) {
		perror("error listen failed");
		close(socketFD);
		exit(EXIT_FAILURE);
	}

	// accept and handle connections
	for(;;) {
		int connectFD = accept(socketFD, NULL, NULL);
	}

	close(socketFD);
	return 0;
}
