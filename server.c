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
#include <ctype.h>
#include <time.h>

#define BACKLOG 10
const char* slash = "/";
const char* endphrase = "HTTP/1.1";

void fixSpaces(char* old_name)
{
	char new_name[512] = {0};
	char* loc = &new_name[0];
	const char* tmp = old_name;
	while(1)
	{
		const char* p = strstr(tmp, "%20");
		if(p == NULL)
		{
			strcpy (loc, tmp);
			break;
		}
		memcpy(loc, tmp, p-tmp);
		loc += p-tmp;
		memcpy(loc, " ", 1);
		loc ++;
		tmp = p+3;
	}
	strcpy(old_name,new_name);
}

char* get_fn(char* buf)
{
	char* file_name;
	if(buf == NULL)
	{
		perror("empty buffer");
		exit(1);
	}

	char* begin = strstr(buf, slash);
	char* cutoff = strstr(buf, endphrase);
	int len = cutoff - begin -2;
	file_name = (char*)malloc((len)*sizeof(char));
	strncpy(file_name, begin+1, len);

	for(int i = 0; i < len; i++)
	{
		file_name[i] = tolower(file_name[i]);
	}
	file_name = fixSpaces(file_name);
	return file_name;
}

void req(int fd)
{
	char* file_name;
	char* file_type;
	char buf[1023];
	char resp[256];
	memset(buf,0,sizeof(buf));
	//read
	if(read(fd, buf, sizeof(buf)) == -1)
	{
		perror("error reading");
		close(fd);
		exit(1);
	}

	
	printf("HTTP Message: \n%s\n", buf);
	if(strlen(buf) == 0)
		return;
	//parse request
	file_name = get_fn(buf);
	if(strstr(file_name, ".jpg") != NULL)
		file_type = "image/jpeg";
	else if(strstr(file_name, ".jpeg") != NULL)
		file_type = "image/jpeg";
	else if(strstr(file_name, ".png") != NULL)
		file_type = "image/png";
	else if(strstr(file_name, ".gif") != NULL)
		file_type = "image/gif";
	else if(strstr(file_name, ".html") != NULL)
		file_type = "text/html";
	else if(strstr(file_name, ".htm") != NULL)
		file_type = "text/html";
	else if(strstr(file_name, ".txt") != NULL)
		file_type = "text/plain";
	else
		file_type = "application/x-binary";
	//printf("%s\n", file_type);

	FILE *fp = fopen(file_name,"r");
	if(fp == NULL)
	{
		//file not found 
		char* error_header = "HTTP/1.1 404 Not Found \r\n";
		write(fd, error_header, strlen(error_header));

		//date
		time_t now;
		time(&now);
		sprintf(resp, "Date: %s\r\n\r\n", ctime(&now));
		write(fd, resp, strlen(resp));



	}
	else
	{
		
		char* succ_header = "HTTP/1.1 200 OK\r\n";
		write(fd, succ_header, strlen(succ_header));
		printf("%s\n", succ_header);
		//file size
		fseek(fp, 0L, SEEK_END);
		int size = (int) ftell(fp);
		fseek(fp, 0L, SEEK_SET);
		sprintf(resp,"Content-Length: %i\r\n", size);
		write(fd, resp, strlen(resp));

		//date
		time_t now;
		time(&now);
		sprintf(resp, "Date: %s\r", ctime(&now));
		write(fd, resp, strlen(resp));

		sprintf(resp, "Content-Type: %s\n\n", file_type);
		write(fd, resp, strlen(resp));

		if(ferror(fp))
		{
			printf("yikes\n");
		}

		char* fbuf = malloc(sizeof(char)* size);
		fread(fbuf,1, size, fp);
		write(fd, fbuf, size);
		fclose(fp);
		free(fbuf);
	}



	free(file_name);
}

int main(int argc, char *argv[])
{
	int sock_fd, new_fd; //file descriptors
	int portno = 8000; //port number
	struct sockaddr_in srv_addr; //socket address server
	struct sockaddr_in cli_addr; //socket address client
	socklen_t sin_size;
	struct sigaction sigact; //for client process

	//get port number argument
	portno = atoi(argv[1]);
	if(portno >= 0 && portno <= 1023)
	{
		perror("invalid portno");
		exit(1);
	}

	//create socket
	if((sock_fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("socket creation error");
		exit(1);
	}

	//clear server address
	memset((char*) &srv_addr, 0, sizeof(srv_addr));


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

	while(1) //main accept() loop
	{
		sin_size = sizeof(struct sockaddr_in);
		new_fd = accept(sock_fd,(struct sockaddr*) &cli_addr, &sin_size);

		if(new_fd == -1)
		{
			perror("failed accept");
			exit(1);
		}

		req(new_fd);
		printf("server: got connection from %s\n", inet_ntoa(cli_addr.sin_addr));
		close(new_fd);
		
	}
  	return 0;
}

