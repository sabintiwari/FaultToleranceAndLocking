#	Sabin Raj Tiwari
#	CMSC 621
#	Project 3

compile: client server backend directories

client:
	g++ -o client client.cpp

server:
	g++ -o server server.cpp -lpthread

backend:
	g++ -o backend backend.cpp

clean:
	rm -f client server backend
	rm -rf logs

directories:
	mkdir -p logs