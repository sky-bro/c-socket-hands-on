target all:
	gcc server.c -o server -lpthread
	gcc client.c -o client
clean:
	rm server client
