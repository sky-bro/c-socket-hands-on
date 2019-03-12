// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#define PORT 8080

int main(int argc, char const *argv[])
{
	printf("argument count: %d\n", argc);
	for (int i = 0; i < argc; i++){
		printf("argument %d: %s\n", i, argv[i]);
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
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char *hello = "Hello from client";
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
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}
	send(sock , hello , strlen(hello) , 0 );
	printf("Hello message sent\n");
	valread = read( sock , buffer, 1024);
	printf("valread: %d\n", valread);
	printf("%s\n",buffer );
	return 0;
}
