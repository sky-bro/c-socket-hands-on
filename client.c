// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "lock_set.c"
// #define PORT 8080
#define BUFFER_SIZE 1024

unsigned long long recvSize(int sock){
	unsigned long long length;
	read(sock, (char *)&length, sizeof(unsigned long long));
	return length;
}

void sendSize(int sock, unsigned long long length){
	write(sock, (char *)&length, sizeof(unsigned long long));
}

int main(int argc, char const *argv[]){
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
	int ch;
	int isUpload = 1;
	char* action = "u";
	// file name
	// char* filename = "1.txt";
	char* local_filename = "2.jpeg";
	// char *filename = "1.mp4";
	char* remote_filename = "0.jpeg";
	char* serverIP = "127.0.0.1";
	int PORT = 8080;
	opterr=0;
	while ((ch = getopt(argc, argv, "u:dl:i:p:r:h")) != EOF /*-1*/) {
		// printf("optind: %d\n", optind);
   	switch (ch){
	       case 'u':
	               break;
	       case 'd':
				 				 isUpload = 0;
	               break;
	       case 'l':
				 				 local_filename = optarg;
								 printf("l: %s", optarg);
	               break;
	       case 'i':
	               serverIP = optarg;
	               break;
	       case 'p':
								 PORT = atoi(optarg);
								 break;
				 case 'r':
				 				 remote_filename = optarg;
								 printf("r: %s", optarg);
	               break;
				 default:
				 				printf("%s", cmds);
				 				exit(1);
		}
	}
	if (!isUpload) {
		action = "d";
	}

	// struct sockaddr_in {
	//
	// 　　short int sin_family; /* 通信类型 */
	//
	// 　　unsigned short int sin_port; /* 端口 */
	//
	// 　　struct in_addr sin_addr; /* Internet 地址 */
	//
	// 　　unsigned char sin_zero[8]; /* 与sockaddr结构的长度相同*/
	//
	// };

	struct sockaddr_in address;
	int sock = 0;
	struct sockaddr_in serv_addr;
	char buffer[1024] = {0};
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	// host to network short
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, serverIP, &serv_addr.sin_addr)<=0)
	// if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}

	// send action: upload or download
	unsigned long long action_len = strlen(action) + 1;
	sendSize(sock, action_len);
	write(sock, action, strlen(action)+1);

	// upload file:
	if (isUpload){
		printf("Preparing sending file: %s\n", local_filename);
		// int fd = open(local_filename, O_RDONLY);
		int fd;
		if((fd=open(local_filename, O_RDONLY)) < 0)//打开操作不成功
		{
				perror("open file failed");
				exit(EXIT_FAILURE);
		}
		// file size
		struct stat statbuf;
	  stat(local_filename,&statbuf);
	  unsigned long long filesize=statbuf.st_size;
		// printf("%s filesize: %d", filename, size);
		printf("send %s of size %lld bytes to the server\n", local_filename, filesize);

		// send filename
		unsigned long long path_len = strlen(remote_filename) + 1;
		sendSize(sock, path_len);
		printf("writing remote_filename: %s to sock", remote_filename);
		write(sock, remote_filename, path_len);
		// send filesize +1?
		// write(sock, (char*)&filesize, sizeof(unsigned long long));
		sendSize(sock, filesize);
		// memset(buffer, 0, sizeof(buffer));
		ssize_t singleRead = 0;
		double sentsize = 0;
		lock_set(fd, F_RDLCK);
		do {
				singleRead = read(fd, buffer, sizeof(buffer));
				// with 0 flag, equivalent to write
				if(singleRead > 0 && send(sock, buffer, singleRead, 0) >= 0 ){
					sentsize += singleRead;
					printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
					printf("%-4.2f%% data sent", sentsize/filesize*100);
					fflush(stdout);
				} else {
					break;
				}
				memset( buffer,0, sizeof(buffer) );
		} while (singleRead > 0);
		lock_set(fd, F_UNLCK);
		close(fd);
	} else {
		// Download from server
		printf("Preparing recving file: %s from server, save to %s\n", remote_filename, local_filename);
		// int fd = open(local_filename, O_WRONLY);
		int fd;
		if((fd=open(local_filename, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) < 0)//打开操作不成功
		{
				perror("open file failed");
				exit(EXIT_FAILURE);
		}

		// send filename
		unsigned long long path_len = strlen(remote_filename) + 1;
		sendSize(sock, path_len);
		printf("writing remote_filename: %s to sock", remote_filename);
		write(sock, remote_filename, path_len);

		unsigned long long filesize = recvSize(sock);

		unsigned long long bytesRecvd = 0;
		// write lock
		lock_set(fd, F_WRLCK);
		do {
				memset(buffer, 0, BUFFER_SIZE);
				int singleRecvd = read(sock, buffer, sizeof(buffer));
				write(fd, buffer, singleRecvd); // return number of bytes written
				bytesRecvd += singleRecvd;
				printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
				printf("%-4.2f%% data recv", bytesRecvd*100.0/filesize);
				fflush(stdout);
		} while (filesize - bytesRecvd > 0);
		lock_set(fd, F_UNLCK);
		close(fd);
	}

	printf("\n");
	close(sock);
	return 0;
}
