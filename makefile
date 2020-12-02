all: server.c client.c
	gcc server.c -o pyserver
	gcc client.c -o client

clean: 
	rm pyserver client
