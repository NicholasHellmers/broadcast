serv:
	gcc -o ./server/server ./server/server.c
	./server/server

cli:
	gcc -o ./client/client ./client/client.c
	./client/client

clean:
	rm -f ./server/server
	rm -f ./client/client

bench: # Simulate 100 clients requests, close once this is done
	gcc -o ./client/client ./client/client.c; \
	echo "Starting 1000 clients"; \
	for i in {1..10000}; do \
		./client/client; \
	done; \
	echo "Done"; \

	