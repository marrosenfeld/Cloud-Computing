all : client server
	
client : client.c
	gcc -Wall -o client client.c -pthread -g

server : server.c 
	gcc -Wall -o server server.c -pthread -g

clean: 
	$(RM) client server