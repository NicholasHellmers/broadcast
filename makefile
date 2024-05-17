serv:
	gcc -o ./server/server ./server/server.c
	./server/server

cli:
	gcc -o ./client/client ./client/client.c
	./client/client

clean:
	rm -f ./server/server
	rm -f ./client/client

test:
	gcc -o ./server/server ./server/server.c
	gcc -o ./client/client ./client/client.c
	./server/server &
	./client/client
	