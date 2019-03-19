// #define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "lock_set.c"
#define BUFFER_SIZE 1024
// #define PORT 8080

char buffer[BUFFER_SIZE] = {0};

unsigned long long recvSize(int sock){
	unsigned long long length;
	read(sock, (char *)&length, sizeof(unsigned long long));
	return length;
}

void sendSize(int sock, unsigned long long length){
	write(sock, (char *)&length, sizeof(unsigned long long));
}

void handleUpload(void* p_new_socket){
	char buffer[BUFFER_SIZE] = {0};
	long new_socket = (long)(p_new_socket);
	// send filename
	memset(buffer, 0, BUFFER_SIZE);
	unsigned long long path_len = recvSize(new_socket);
	read(new_socket, buffer, path_len);
	printf("filename: %s\n", buffer);
	unsigned long long filesize = recvSize(new_socket);

	// uid_t ruid ,euid,suid;
	// getresuid(&ruid, &euid, &suid);
	// printf("resuid: (%u, %u, %u)", ruid, euid, suid);

	// user: rw, group/other: r
	int fd;
	if((fd=open(buffer, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) < 0)//打开操作不成功
	{
			perror("open file failed");
			exit(EXIT_FAILURE);
	}

	printf("prepared file %s for upload\n", buffer);

	// printf("Recving data of size %lld bytes from client\n", filesize);
	unsigned long long bytesRecvd = 0;
	// write lock
	lock_set(fd, F_WRLCK);
	do {
			memset(buffer, 0, BUFFER_SIZE);
			int singleRecvd = read(new_socket, buffer, sizeof(buffer));
			write(fd, buffer, singleRecvd); // return number of bytes written
			bytesRecvd += singleRecvd;
			// printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
			// printf("%-4.2f%% data recv", bytesRecvd*100.0/filesize);
			// fflush(stdout);
	} while (filesize - bytesRecvd > 0);
	// } while (1);
	// unlock
	lock_set(fd, F_UNLCK);
	close(fd);
	close(new_socket);
	printf("---------------------\n");
}

void handleDownload(void* p_new_socket){
	char buffer[BUFFER_SIZE] = {0};
	long new_socket = (long)(p_new_socket);
	// send filename
	memset(buffer, 0, BUFFER_SIZE);
	unsigned long long path_len = recvSize(new_socket);
	read(new_socket, buffer, path_len);
	printf("filename: %s\n", buffer);



	// uid_t ruid ,euid,suid;
	// getresuid(&ruid, &euid, &suid);
	// printf("resuid: (%u, %u, %u)", ruid, euid, suid);

	// user: rw, group/other: r
	int fd;
	if((fd=open(buffer, O_RDONLY)) < 0)//打开操作不成功
	{
			perror("open file failed");
			exit(EXIT_FAILURE);
	}

	printf("prepared file %s for download\n", buffer);

	// send file size
	struct stat statbuf;
  stat(buffer,&statbuf);
  unsigned long long filesize=statbuf.st_size;
	sendSize(new_socket, filesize);

	printf("send %s of size %lld bytes to the client\n", buffer, filesize);

	ssize_t singleRead = 0;
	double sentsize = 0;
	lock_set(fd, F_RDLCK);
	do {
			singleRead = read(fd, buffer, sizeof(buffer));
			// with 0 flag, equivalent to write
			if(singleRead > 0 && send(new_socket, buffer, singleRead, 0) >= 0 ){
				sentsize += singleRead;
			} else {
				break;
			}
			memset( buffer,0, sizeof(buffer) );
	} while (singleRead > 0);
	lock_set(fd, F_UNLCK);
	close(fd);
	close(new_socket);
	printf("---------------------\n");
}

int main(int argc, char const *argv[])
{
		char *cmds = "Always fire up server first.\n\
default port 8080\n\n\
Upload file to server\n\
----\n\
server: ./server [-p port]\n\
client: ./client -u -l path/on/client -i serverIP [-p port] -r path/on/server\n\n\
Download file from server\n\
----\n\
server: ./server [-p port]\n\
client: ./client -d -l path/on/client -i serverIP [-p port] -r path/on/server\n";
	// printf("%s", cmds);
	// exit(0);
	int PORT = 8080;
	int maxClient = 5;
	opterr = 0;
	char ch;
		while ((ch = getopt(argc, argv, "p:m:")) != EOF /*-1*/) {
		// printf("optind: %d\n", optind);
   	switch (ch){
	       case 'p':
								 PORT = atoi(optarg);
								 break;
				 case 'm':
								 maxClient = atoi(optarg) < 100? atoi(optarg):maxClient;
								 break;
				 default:
				 				printf("%s", cmds);
				 				exit(1);
		}
	}

	chdir("./serverFile");
	int server_fd, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);

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
	if (listen(server_fd, maxClient) < 0)
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
		printf("***Connected to client: %s@%d***\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
		// send action: upload or download
		unsigned long long action_len = recvSize(new_socket);
		read(new_socket, buffer, action_len);
		int isUpload = 1;
		printf("*Action: %s\n", buffer);
		if (strcmp(buffer, "u")){
			isUpload = 0;
		}
		// if (isUpload){
		// 	printf("upload file\n");
		// } else {
		// 	printf("download file\n");
		// }
	// handle upload file
		pthread_t thread;
		int ret_thrd;
		if (isUpload){
			printf("upload file to me\n");
			ret_thrd = pthread_create(&thread, NULL, (void *)&handleUpload, (void *)new_socket);
		} else {
			printf("download file from me\n");
			ret_thrd = pthread_create(&thread, NULL, (void *)&handleDownload, (void *)new_socket);
		}

		if (ret_thrd) { // != 0
         printf("create handle thread failed\n");
     } else { // = 0
         printf("create handle thread success\n");
     }
	}
	close(server_fd);
	return 0;
}
