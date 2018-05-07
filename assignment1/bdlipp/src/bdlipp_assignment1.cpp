/**
 * @bdlipp_assignment1
 * @author  Benjamin Lipp <bdlipp@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <list>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/poll.h>


using namespace std;


#include "../include/global.h"
#include "../include/logger.h"
char input[1024];
char IP[256];
#define MAXDATASIZE 100 // max number of bytes we can get at once
#define BACKLOG 10	 // how many pending connections queue will hold
char PORT[1024];
void sigchld_handler(int s){
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}
void *get_in_addr(struct sockaddr *sa){
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int getip(char *hostname){

	struct addrinfo hints, *res, *p;
	int status;
	char ipstr[INET6_ADDRSTRLEN];
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
	hints.ai_socktype = SOCK_STREAM;
	if ((status = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
			return 2;
	}
	for(p = res;p != NULL; p = p->ai_next) {
			void *addr;
			const char *ipver;
			// get the pointer to the address itself,
			// different fields in IPv4 and IPv6:
			if (p->ai_family == AF_INET) { // IPv4
					struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
					addr = &(ipv4->sin_addr);
					ipver = "IPv4";
			} else { // IPv6
					struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
					addr = &(ipv6->sin6_addr);
					ipver = "IPv6";
			}
			// convert the IP to a string and print it:
			inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
			strcpy(IP,ipstr);

	}
	freeaddrinfo(res); // free the linked list
	return 0;
}


int start_server(void){

	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	list<string> iplist;
	list<string> hostlist;
	list<string>::iterator it;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %i\n", rv);
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);
	getip(hostname);
	cin >> input;
	while(strcmp(input,(char *)"EXIT")!=0){
		if(strcmp(input,(char *)"AUTHOR")==0){
			cse4589_print_and_log((char *)"[%s:SUCCESS]\n", "AUTHOR");
			cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", "bdlipp");
			cse4589_print_and_log((char *)"[%s:END]\n", "AUTHOR");
		}
		else if(strcmp(input,(char *)"PORT")==0){
			cse4589_print_and_log((char *)"[%s:SUCCESS]\n", "PORT");
			cse4589_print_and_log("PORT:%s\n", PORT);
			cse4589_print_and_log((char *)"[%s:END]\n", "PORT");
		}
		else if(strcmp(input,(char *)"IP")==0){
			cse4589_print_and_log((char *)"[%s:SUCCESS]\n", "IP");
			cse4589_print_and_log("IP:%s\n", IP);
			cse4589_print_and_log((char *)"[%s:END]\n", "IP");
		}
		else if(strcmp(input,(char *)"LIST")==0){
			for (it=iplist.begin(); it!=iplist.end(); ++it)
				cse4589_print_and_log(*it);
		}
		else if(strcmp(input,(char *)"EXIT")==0){
			break;
		}
		fd_set read_fds;
		FD_ZERO(&read_fds);
		FD_SET(sockfd, &read_fds);

		struct timeval timeout;
		timeout.tv_sec = 1;  // 1s timeout
		timeout.tv_usec = 0;

		int select_status;
		select_status = select(sockfd+1, &read_fds, NULL, NULL, &timeout);
		if (select_status == -1) {
				// ERROR: do something
		} else if (select_status > 0) {
		sin_size = sizeof their_addr;
			new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
			if (new_fd == -1) {
				perror("accept");
			}
			inet_ntop(their_addr.ss_family,
				get_in_addr((struct sockaddr *)&their_addr),
				s, sizeof s);
				struct hostent *he;
			struct in_addr ipv4addr;
			struct in6_addr ipv6addr;

			inet_pton(AF_INET, s, &ipv4addr);
			he = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
			printf("server: got connection from %s\n", he->h_name);
			iplist.push_back(he->h_name);
		}
		// otherwise (i.e. select_status==0) timeout, continue






		cin >> input;

	}
	return 0;
}
int start_client(void){
	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);
	getip(hostname);
	cin >> input;
	while(strcmp(input,(char *)"EXIT")!=0){
		if(strcmp(input,(char *)"AUTHOR")==0){
			cse4589_print_and_log((char *)"[%s:SUCCESS]\n", "AUTHOR");
			cse4589_print_and_log("I, %s, have read and understood the course academic integrity policy.\n", "bdlipp");
			cse4589_print_and_log((char *)"[%s:END]\n", "AUTHOR");
		}
		else if(strcmp(input,(char *)"PORT")==0){
			cse4589_print_and_log((char *)"[%s:SUCCESS]\n", "PORT");
			cse4589_print_and_log("PORT:%s\n", PORT);
			cse4589_print_and_log((char *)"[%s:END]\n", "PORT");
		}
		else if(strcmp(input,(char *)"IP")==0){
			cse4589_print_and_log((char *)"[%s:SUCCESS]\n", "IP");
			cse4589_print_and_log("IP:%s\n", IP);
			cse4589_print_and_log((char *)"[%s:END]\n", "IP");
		}
		else if(strncmp(input,(char *)"LOGIN",5)==0){
			int sockfd, numbytes;
			char buf[MAXDATASIZE];
			struct addrinfo hints, *servinfo, *p;
			int rv;
			char s[INET6_ADDRSTRLEN];
			memset(&hints, 0, sizeof hints);
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;

			if ((rv = getaddrinfo("stones.cse.buffalo.edu", PORT, &hints, &servinfo)) != 0) {
				fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
				return 1;
			}
			// loop through all the results and connect to the first we can
			for(p = servinfo; p != NULL; p = p->ai_next) {
				if ((sockfd = socket(p->ai_family, p->ai_socktype,
						p->ai_protocol)) == -1) {
					perror("client: socket");
					continue;
				}
				if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
					perror("client: connect");
					close(sockfd);
					continue;
				}
				break;
			}
			if (p == NULL) {
				fprintf(stderr, "client: failed to connect\n");
				return 2;
			}
			inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
					s, sizeof s);
			printf("client: connecting to %s\n", s);
			freeaddrinfo(servinfo); // all done with this structure
			if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
					perror("recv");
					exit(1);
			}
			buf[numbytes] = '\0';
			printf("client: received '%s'\n",buf);
			close(sockfd);
		}
		else if(strcmp(input,(char *)"EXIT")==0){
			break;
		}
		cin >> input;

	}


	return 0;
}

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/* Clear LOGFILE*/
    fclose(fopen(LOGFILE, "w"));
	if(argc != 3){
		cse4589_print_and_log("no\n");
		return 1;
	}
	/*Start Here*/

	strcpy(PORT,argv[2]);
	int a;
	if(*argv[1]=='c'){
		a=start_client();
	}
	else if(*argv[1]=='s'){
		a=start_server();
	}
	else{
		cse4589_print_and_log("no\n");
		return 1;
	}
	return 0;
}
