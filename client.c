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
// #define PORT 8080


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
	while ((ch = getopt(argc, argv, "u:d:l:i:p:r:h")) != EOF /*-1*/) {
		// printf("optind: %d\n", optind);
   	switch (ch){
	       case 'u':
	               break;
	       case 'd':
				 				 isUpload = 0;
	               break;
	       case 'l':
				 				 local_filename = optarg;
	               break;
	       case 'i':
	               serverIP = optarg;
	               break;
	       case 'p':
								 PORT = atoi(optarg);
								 break;
				 case 'r':
				 				 remote_filename = optarg;
	               break;
				 default:
				 				printf("%s", cmds);
				 				exit(1);
		}
	}
	if (!isUpload) {
		action = "d";
	}
	// if (argc != )
	// printf("argument count: %d\n", argc);
	// for (int i = 0; i < argc; i++){
	// 	printf("argument %d: %s\n", i, argv[i]);
	// }

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
	char *hello = "Hello from client";
	char buffer[1024] = {0};
	char res[2];
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

// download:
// file name

// file size

// file

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}
	// send & recv Hello String
	// send(sock , hello , strlen(hello) , 0 );
	// printf("Hello message sent\n");
	// read( sock , buffer, 1024);
	// printf("%s\n",buffer );

	// upload file:

	printf("Preparing sending file: %s\n", local_filename);
	int fd = open(local_filename, O_RDONLY);
	// file size
	struct stat statbuf;
  stat(local_filename,&statbuf);
  unsigned long long filesize=statbuf.st_size;
	// printf("%s filesize: %d", filename, size);
	printf("send %s of size %lld bytes to the server\n", local_filename, filesize);

	// send action: upload or download
	send(sock, action, strlen(action), 0);
	// send filename
	printf("remote_filename: %s", remote_filename);
	send(sock, remote_filename, strlen(remote_filename)+1, 0);
	// send filesize +1?
	// send(sock, (char*)&filesize, sizeof(unsigned long long), 0);
	// read(sock, buffer, sizeof(buffer));
	// printf("res: %s", buffer);
	memset(buffer, 0, sizeof(buffer));
	ssize_t singleRead = 0;
	double sentsize = 0;
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
	printf("\n");
	close(fd);
	close(sock);
	return 0;
}
