//Assignment Project Chat Ipv4
//Client code

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

//Server's port number
#define SERVPORT 2020

//BufferLength is 100 bytes
#define BufferLength 100

//Variable and structure definitions.
pthread_t scanf_thread1_v, recv_thread2_v;
int sd; //socket descriptor
struct sockaddr_in serveraddr; //Server address

void * scanf_thread1_f() {
	int rc; 
	unsigned int length = sizeof(int);
	char txbuffer[BufferLength];
	char temp;

	while (1) {
		
		rc = scanf("%[^\n]", txbuffer);

		getc(stdin);

		//Check if it is "exit"
		if (memcmp(txbuffer, "exit", 4) == 0) {
			close(sd);
			exit(EXIT_SUCCESS);
			return 0;
		} else {
			//Send data to server
			rc = write(sd, txbuffer, strlen(txbuffer)+1);	
			if (rc < 0) {
				perror("Client-write() error");
				rc = getsockopt(sd, SOL_SOCKET, SO_ERROR, &temp, &length);
				if (rc == 0) {
					//Print out the asynchronously received error.
					errno = temp;
					perror("SO_ERROR was");
				}
				close(sd);
				exit(-1);
			}
		}
	}
	return NULL;
}

void * recv_thread2_f() {
	int rc; 
	char rxbuffer[BufferLength];

	while (1) {
		//Lets check if there is data received on the socket.
		rc = recv(sd, rxbuffer, BufferLength, 0);
		if (rc > 0) {
			//Data Received from Server, so display it
			printf("%s\n", rxbuffer);
		}
	}
	return NULL;
}

//Pass the IP address of the server as the parameter
int main(int argc, char *argv[]) {
	int rc; //Return code
	char server[255]; //Server IP address as passed as command line argument
	struct hostent *hostp;

	printf("Client starting pid=%d...\n",getpid());
	
	if(argc <2) {
		printf("Please specify server IP address as a command line argument!\n");
		exit(-1);
	}
	//STEP01: Create Socket
	//The socket() function returns a socket descriptor representing an endpoint.
	//The statement also identifies that the INET (Internet Protocol) address family
	//with the TCP transport (SOCK_STREAM) will be used for this socket.
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Client-socket() error");
		exit(-1);
	} else
		printf("Client-socket() OK\n");

	//STEP02: Resolve Server Address
	strcpy(server, argv[1]);
	printf("Connecting to %s, port %d ...\n", server, SERVPORT);
	memset(&serveraddr, 0x00, sizeof(struct sockaddr_in));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVPORT);

		//When passing the host name of the server as a parameter to this program,
		//use the gethostbyname() function to retrieve the address of the host server.
		hostp = gethostbyname(server);
		if (hostp == (struct hostent *) NULL) {
			printf("HOST NOT FOUND --> ");
			//h_errno is usually defined in netdb.h
			printf("h_errno = %d\n", h_errno);
			printf("---This is a client program---\n");
			printf("Command usage: %s <server name or IP>\n", argv[0]);
			close(sd);
			exit(-1);
		}
		memcpy(&serveraddr.sin_addr, hostp->h_addr, sizeof(serveraddr.sin_addr));


	//STEP03: TCP Connect
	//After the socket descriptor is received, the connect() function is used to establish a connection to the server.
	if ((rc = connect(sd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)))
			< 0) {
		perror("Client-connect() error");
		close(sd);
		exit(-1);
	} else
		printf("Connection established...\n");

	//Put socket in non-blocking mode, so that when we call the recv function, it does not block.
	fcntl(sd, F_SETFL, O_NONBLOCK);

	//Start threads
	pthread_create(&scanf_thread1_v, NULL, scanf_thread1_f, NULL);
	pthread_create(&recv_thread2_v, NULL, recv_thread2_f, NULL);

	//Wait for threads to terminate
	pthread_join(scanf_thread1_v, NULL);
	pthread_join(recv_thread2_v, NULL);

	close(sd);
	exit(EXIT_SUCCESS);
	return 0;
}
