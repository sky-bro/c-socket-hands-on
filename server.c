// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 8080
int main(int argc, char const *argv[])
{
	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = {0};
	char *hello = "Hello from server";

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
												&opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr *)&address,
								sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	// maximum client
	if (listen(server_fd, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
					(socklen_t*)&addrlen))<0)
	{
		perror("accept");
		exit(EXIT_FAILURE);
	}
// send hello string
	// valread = read( new_socket , buffer, 1024);
	// printf("%s\n",buffer );
	// send(new_socket , hello , strlen(hello) , 0 );
	// printf("Hello message sent\n");
// handle upload file

// filename
	// char *filename = "0.txt";
	// char *filename = "0.jpeg";
	char *filename = "0.mp4";
	// FILE *fp = fopen(filename, "w+");
	//定义一个名叫fp文件指针
	FILE *fp;
	//判断按读方式打开一个名叫test的文件是否失败
	if((fp=fopen(filename, "w+")) == NULL)//打开操作不成功
	{
	    printf("The file can not be opened.\n");
	    exit(1);//结束程序的执行
	}
	printf("prepared file %s for upload\n", filename);
// filesize
	unsigned long long filesize = 0;
	recv(new_socket, (char*)&filesize, sizeof(unsigned long long)+1, 0);
	printf("Recving data of size %lld bytes from client\n", filesize);
	unsigned long long bytesRecvd = 0;
// file
	do {
			int singleRecvd = recv(new_socket, buffer, sizeof(buffer),0);
			fwrite(buffer, singleRecvd, 1, fp);
			bytesRecvd += singleRecvd;
			printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
			printf("%-4.2f%% data recv", bytesRecvd*100.0/filesize);
			fflush(stdout);
	} while (filesize - bytesRecvd > 0);
// handle download file
// filename
// filesize
// file
	printf("\n");
	fclose(fp);
	close(new_socket);
	close(server_fd);
	return 0;
}
