ser: server.cpp client.cpp bserver.cpp
	g++ -std=c++11 server.cpp -lrpc -lpthread -o server
	g++ -std=c++11 client.cpp -lrpc -lpthread -o client
	g++ -std=c++11 bserver.cpp -lrpc -lpthread -o bserver
	mkdir backup_server
	cp bserver ./backup_server/bserver
	rm bserver

.PHONY: clean

clean: client server backup_server
	
	rm ./client
	rm ./server
	rm -r ./backup_server

