#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define BACKLOG 10

void sighand()
{
	while(waitpid(-1,NULL,WNOHANG) >0) //wait until child process terminates
}


int main(int argc, char *argv[])
{
	int sock_fd, new_fd; //file descriptors
	int portno = 8000; //port number
	struct sockaddr_in srv_addr; //socket address server
	struct sockaddr_in cli_addr; //socket address client
	socklen_t sin_size;
	struct sigaction sigact; //for client process
	pid_t pid; //process id


	//create socket
	if((sock_fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("socket creation error");
		exit(1);
	}

	//clear server address
	memset((char*) &srv_addr, 0, sizeof(srv_addr));

	//get port number argument
	portno = atoi(argv[1]);

	//set address info
	srv_addr.sin_family = AF_INET;;
	srv_addr.sin_port = htons(portno);
	srv_addr.sin_addr.s_addr = INADDR_ANY;

	//bind socket
	if(bind(sock_fd,(struct sockaddr*) &srv_addr, sizeof(struct sockaddr)) == -1)
	{
		perror("binding error");
		exit(1);
	}

	//listen, put socket into passive state 
	if(listen(sock_fd, BACKLOG) == -1)
	{
		perror("listen error");
		exit(1);
	}

	//preprae for child process
	sigact.sa_handler = sighand;
	sigemptyset(&sa.sa_mask);
	sigact.sa_flags = SA_RESTART;
	if(sigaction(SIGCHLD,&sigact, NULL) == -1)
	{
		perror("error in setting up child proc");
		exit(1);
	}

	while(1) //main accept() loop
	{
		sin_size = sizeof(struct sockaddr_in);
		new_fd = accept(sock_fd,(struct sockaddr*) &cli_addr, &sin_size)

		if(new_fd == -1)
		{
			perror("failed accept");
			exit(1);
		}

		pid = fork()

		if(pid == -1)
		{
			perror("failure fork");
			exit(1);
		}

		if(pid == 0) //child process
		{
			close(sock_fd);
			do_something(); 
			exit(0);
		}
		else //parent process
		{
			printf("server: got connection from %s\n", inet_ntoa(cli_addr.sin_addr));
			close(new_fd);
		}
	}
  	return 0;
}
