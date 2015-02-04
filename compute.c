/***************************************************
 * compute - Computes Perfect Numbers
 * Hugh McDonald
 * 12.6.13
 ***************************************************/
#define _POSIX_C_SOURCE 200809L
#define _BSD_SOURCE

//IP ADDRESS OF SERVER
#define IP "128.193.37.168"
#define PORT 7070
#define MAXLINE 4096

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/*
 * Function declarations
 */
int main(int argc, char *argv[]);
unsigned long benchmark();
void *siglisten(void *arg);
int perfect_helper(unsigned long start, unsigned long end);
int compute_perfect(unsigned long performance);
int report_perfect(unsigned long number);
int recv_data(int sockfd, char *data);
int send_data(int sockfd, char *data);

/*
 * Gets range to compute from server and checks for perfect numbers
 */
int compute_perfect(unsigned long performance)
{
	//create send message
	char sbuf[1024];
	memset(&sbuf, 0, sizeof(sbuf));
	strcpy(sbuf, "<request>");
	char perf[12];
	sprintf(perf, "%lu", performance);
	strcat(sbuf, perf);
	strcat(sbuf, "</request>");

	//server info
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr)); 
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	inet_pton(AF_INET, IP, &servaddr.sin_addr.s_addr);

	while (1) {
		//connect to server
		int sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1)
			return 1;

		//send request
		send_data(sockfd, sbuf);

		//read response
		char rbuf[1024];
		memset(rbuf, 0, sizeof(rbuf));
		recv_data(sockfd, rbuf);	
		
		//close connection
		close(sockfd);

		//parse response
		char *string;
		string = strtok(rbuf, "</>");
		string = strtok(NULL, "</>");
		unsigned long start = strtol(string, NULL, 10);
		string = strtok(NULL, "</>");
		string = strtok(NULL, "</>");
		string = strtok(NULL, "</>");
		unsigned long end = strtol(string, NULL, 10);
		string = strtok(NULL, "</>");

		//compute perfect numbers
		perfect_helper(start, end);
	}
	return 0;
}

/*
 * Computes perfect numbers
 */
int perfect_helper(unsigned long start, unsigned long end)
{
	for (int i = start; i <= end; ++i) {
		unsigned long sum = 0;
		for (int j = 1; j < i; ++j) {
			if (i % j == 0)
				sum += j;
		}
		if (sum == i)
			report_perfect(i);
	}
	return 0;
}

/*
 * Reports perfect numbers
 */
int report_perfect(unsigned long number)
{
	//create send message
	char sbuf[1024];
	memset(&sbuf, 0, sizeof(sbuf));
	strcpy(sbuf, "<perfect>");
	char num[12];
	sprintf(num, "%lu", number);
	strcat(sbuf, num);
	strcat(sbuf, "</perfect>");

	//server info
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr)); 
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	inet_pton(AF_INET, IP, &servaddr.sin_addr.s_addr);

	//connect to server
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
		printf("compute: server not found @ %s\n", IP);
		exit(1);
	}

	//send the message
	send_data(sockfd, sbuf);
	
	//close connection
	close(sockfd);

	return 0;
}

/*
 * Recieves data from a socket
 */
int recv_data(int sockfd, char *data)
{
	//create buffers
	char buf[1024];
	memset(buf, 0, sizeof(buf));
	char header[5];
	memset(header, 0, sizeof(header));
	char headbuf[5];
	memset(headbuf, 0, sizeof(headbuf));

	//recieve header
	int numread = 0;
	int recieved = 0;
	int target = 5;
	while (recieved < target) {
		numread = read(sockfd, headbuf, target - recieved);
		strcat(header, headbuf);
		recieved += numread;
	}
	int size = ntohs(atoi(header));

	//recieve message
	recieved = 0;
	while (recieved < size) {
		numread = read(sockfd, buf, size - recieved);
		strcat(data, buf);
		recieved += numread;
	}
	return 0;
}

/*
 * Sends data to a socket
 */
int send_data(int sockfd, char *data)
{
	//create header
	char header[5];
	memset(header, 0, sizeof(header));
	int msglen = strlen(data);
	uint16_t size = htons((uint16_t)msglen);
	sprintf(header, "%d", size);
	int sent = 0;

	//send header
	int left = sizeof(header);
	while (left) {
		sent = write(sockfd, header + sizeof(header) - left, left);
		if (sent < 0)
			return 1;
		left -= sent;
	}

	//send message
	left = strlen(data);
	while (left) {
		sent = write(sockfd, data + strlen(data) - left, left);
		if (sent < 0)
			return 1;
		left -= sent;
	}
	return 0;
}
		
/*
 * Benchmarks this client
 */
unsigned long benchmark()
{
	time_t proctime;
	volatile unsigned long dummy1 = 0;
	unsigned long dummy2 = 17;

	time(&proctime);
	for (unsigned long i = 1; i < 1000000000; ++i) {
		dummy1 += dummy2 % i;
	}
	proctime = time(NULL) - proctime;
	
	return 1000000000l / (unsigned long)proctime;
}

/*
 * [threaded] Listens for exit command from server
 */
void *siglisten(void *arg)
{
	unsigned long *performance = (unsigned long *)arg;
	char sbuf[1024];
	char rbuf[1024];
	memset(&sbuf, 0, sizeof(sbuf));
	memset(&rbuf, 0, sizeof(rbuf));

	//create send message
	strcpy(sbuf, "<signal>");
	char perf[12];
	sprintf(perf, "%lu", *performance);
	strcat(sbuf, perf);
	strcat(sbuf, "</signal>");

	//server info
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr)); 
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	inet_pton(AF_INET, IP, &servaddr.sin_addr.s_addr);

	//connect to server
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
		printf("compute: server not found @ %s\n", IP);
		exit(1);
	}

	//send request
	send_data(sockfd, sbuf);

	//when this socket closes we exit
	read(sockfd, rbuf, sizeof(rbuf));

	//exit
	kill(0, SIGTERM);
	pthread_exit(0);
}

/*
 * Signal handling
 */
void sig_hand(int signal)
{
	kill(0, SIGTERM);
}

int main(int argc, char *argv[])
{
	
	//signal handling
	struct sigaction s;
	struct sigaction t;

	s.sa_handler = sig_hand;
	sigemptyset(&s.sa_mask);
	s.sa_flags = 0;

	sigaction(SIGINT, &s, (void *)&t);
	sigaction(SIGQUIT, &s, (void *)&t);
	sigaction(SIGHUP, &s, (void *)&t);

	//benchmark
	unsigned long performance;
	performance = benchmark();

	//open listen thread
	pthread_t thread;
	if (pthread_create(&thread, NULL, siglisten, (void *)&performance) != 0) {
		printf("compute: error creating listen thread\n");
		return 1;
	}

	//compute perfect numbers
	if (compute_perfect(performance)) { 
		printf("compute: server not found @ %s\n", IP);
		return 1;
	}

	return 0;
}
