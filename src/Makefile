#	Sabin Raj Tiwari
#	CMSC 621
#	Project 3

compile:  comm client directories

server:
	g++ -o server server.cpp

comm:
	g++ -o comm comm.cpp -lpthread

client:
	g++ -o client client.cpp

clean:
	rm -f server comm client
	rm -rf logs

directories:
	mkdir -p logs