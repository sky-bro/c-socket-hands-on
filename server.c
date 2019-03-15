// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include "lock_set.c"

#define PORT 8080

char buffer[1024] = {0};

void handleUpload(void* p_new_socket){
	long new_socket = (long)(p_new_socket);
	// filename
		// char *filename = "0.txt";
	char *filename = "icon.png";
	// FILE *fp = fopen(filename, "w+");
	//定义一个名叫fp文件指针
	// FILE *fp;
	int fd;
	// open(filename, O_CREAT|O_WRONLY|O_TRUNC) == create(filename)
	if((fd=open(filename, O_CREAT|O_WRONLY|O_TRUNC)) < 0)//打开操作不成功
	{
			perror("open file failed");
			exit(EXIT_FAILURE);
	}

	printf("prepared file %s for upload\n", filename);
// filesize
	unsigned long long filesize = 0;
	recv(new_socket, (char*)&filesize, sizeof(unsigned long long)+1, 0);
	printf("Recving data of size %lld bytes from client\n", filesize);
	unsigned long long bytesRecvd = 0;
	// write lock
	lock_set(fd, F_WRLCK);
	do {
			int singleRecvd = recv(new_socket, buffer, sizeof(buffer),0);
			write(fd, buffer, singleRecvd); // return number of bytes written
			bytesRecvd += singleRecvd;
			// printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
			// printf("%-4.2f%% data recv", bytesRecvd*100.0/filesize);
			// fflush(stdout);
	} while (filesize - bytesRecvd > 0);
	// unlock
	lock_set(fd, F_UNLCK);
	close(fd);
	close(new_socket);
}

void handleDownload(){

}

int main(int argc, char const *argv[])
{
	int server_fd, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
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
	while (1) {
		// int* p_new_socket = (int*)malloc(sizeof(int));
		long new_socket;
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
						(socklen_t*)&addrlen))<0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}

	// handle upload file
		pthread_t up_thread;
		int ret_thrd1 = pthread_create(&up_thread, NULL, (void *)&handleUpload, (void *)new_socket);
		if (ret_thrd1 != 0) {
         printf("create upload thread failed\n");
     } else {
         printf("create upload thread success\n");
     }
	}
	close(server_fd);
	return 0;
}
