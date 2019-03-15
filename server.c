#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include "lock_set.c"
#define BUFFER_SIZE 1024
#define PORT 8080
#define FILE_SIZE 500

char buffer[BUFFER_SIZE] = {0};

void handleUpload(void* p_new_socket){
	long new_socket = (long)(p_new_socket);
	// send filename
	memset(buffer, 0, BUFFER_SIZE);
	int path_len = read(new_socket, buffer, sizeof(buffer));
	char filename[FILE_SIZE] = {0};
	strncpy(filename, buffer, FILE_SIZE);
	memset(buffer, 0, BUFFER_SIZE);
	printf("filename: %s\n", filename);
	// filesize
	// write(new_socket, "ok", 3);
	// unsigned long long filesize = 0;
	// read(new_socket, (char*)&filesize, sizeof(unsigned long long));
	// printf("filesize: %lld", filesize);
	// filename
		// char *filename = "0.txt";
	// char *filename = "0.jpeg";
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

	// printf("Recving data of size %lld bytes from client\n", filesize);
	unsigned long long bytesRecvd = 0;
	// write lock
	lock_set(fd, F_WRLCK);
	do {
			int singleRecvd = read(new_socket, buffer, sizeof(buffer));
			if (singleRecvd <= 0){
				break;
			}

			write(fd, buffer, singleRecvd); // return number of bytes written
			bytesRecvd += singleRecvd;
			// printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
			// printf("%-4.2f%% data recv", bytesRecvd*100.0/filesize);
			// fflush(stdout);
	// } while (filesize - bytesRecvd > 0);
	} while (1);
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
		// send action: upload or download
		read(new_socket, buffer, sizeof(buffer));
		int isUpload = 1;
		printf("%s\n", buffer);
		if (strcmp(buffer, "u")){
			isUpload = 0;
		}
		if (isUpload){
			printf("upload file\n");
		} else {
			printf("download file\n");
		}
	// handle upload file
		pthread_t thread;
		int ret_thrd;
		if (isUpload){
			ret_thrd = pthread_create(&thread, NULL, (void *)&handleUpload, (void *)new_socket);
		} else {
			ret_thrd = pthread_create(&thread, NULL, (void *)&handleDownload, (void *)new_socket);
		}

		if (ret_thrd != 0) {
         printf("create handle thread failed\n");
     } else {
         printf("create handle thread success\n");
     }
	}
	close(server_fd);
	return 0;
}
