serv:
	gcc -o ./server/server ./server/server.c
	./server/server

cli:
	gcc -o ./client/client ./client/client.c
	./client/client

clean:
	rm -f ./server/server
	rm -f ./client/client

sim_100_cli: # Simulate 100 clients, add i to 3000 for port number
	for i in {3000..3100}; do \
		gcc -o ./client/client ./client/client.c; \
		./client/client & \
	done
	
	