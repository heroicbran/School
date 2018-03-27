rpcp2p: rpcp2p.cc
	g++ -std=c++11 rpcp2p.cc -lrpc -lpthread -o server
	cp server ./peer1/
	mv ./peer1/server ./peer1/peer1
	cp server ./peer2/
	mv ./peer2/server ./peer2/peer2
	cp server ./peer3/
	mv ./peer3/server ./peer3/peer3
	cp server ./peer4/
	mv ./peer4/server ./peer4/peer4

.PHONY: clean

clean: server ./peer1/peer1 ./peer2/peer2 ./peer3/peer3 ./peer4/peer4
	rm server
	rm ./peer1/peer1
	rm ./peer2/peer2
	rm ./peer3/peer3
	rm ./peer4/peer4
