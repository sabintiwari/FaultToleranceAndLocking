/*
	Sabin Raj Tiwari
	CMSC 621
	Project 3
*/

#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

/* Define the global constants. */
#define MAXDATASIZE 1024
#define MAXCONNECTIONS 4

/* Import namespaces. */
using namespace std;

/* Structures for the program. */
struct record
{
	int id, locked;
    double balance;
    pthread_mutex_t lock;
    pthread_cond_t cond;
};
struct server_data
{
	int lfd, sfd, port;
    std::string host;
	struct sockaddr_in address, client;
};

/* Method to construct a record. */
record construct_record(int id, double balance) {
    record data;
    data.id = id;
    data.balance = balance;
    data.locked = 0;
    pthread_cond_init(&data.cond, NULL);
    pthread_mutex_init(&data.lock, NULL);
    return data;
}

/* Global variables. */
std::fstream log_file;
std::vector<record> records;

/* Gets the record with the provided id if it exists. */
record get_record(int id)
{
    for(int i = 0; i < records.size(); i++)
    {
        if(records.at(i).id == id) 
        {
            return records.at(i);
        }
    }
    return construct_record(-1, 0.00);
}

/* Handle the create request from the coordinator. */
void create()
{

}

/*  Main method that handles the client program logic. */
int main(int argc, char **argv)
{
    if(argc < 2)
    {
        /* Show error if the correct number of arguments were not passed. */
		cerr << "Usage: backend <port_number>\n";
		exit(1);
    }

    /* Create the stream socket for the backend server. */
    server_data data;
    data.lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(data.lfd < 0)
    {
        cerr << "Error creating socket.\n";
        exit(1);
    }

    /* Declare the socket data and server address. */
    data.port = atoi(argv[1]);
    data.address.sin_family = AF_INET;
    data.address.sin_addr.s_addr = INADDR_ANY;
    data.address.sin_port = htons(data.port);

    /* Bind the server address to the listen file descriptor. */
    if(bind(data.lfd, (struct sockaddr *)&(data.address), sizeof(data.address)) < 0)
    {
        cerr << "Error binding socket.\n";
        exit(1);
    }

    /* Listen for the incoming requests. */
    cout << "Back-end server waiting for front-end server requests...\n";
    listen(data.lfd, MAXCONNECTIONS);

    /* Accept a client request. */
    socklen_t addrlen = sizeof(data.client);
    data.sfd = accept(data.lfd, (struct sockaddr *)&(data.client), &addrlen);
    if(data.sfd < 0)
    {
        cerr << "Error accepting client request.\n";
        exit(1);
    }

    /* Values to store the responses. */
    char buffer[MAXDATASIZE];
    string buffer_str, client_addr;
    client_addr = inet_ntoa(data.client.sin_addr);
    int status;

    /* Keep reading in data. */
    while(1)
    {
        /* Read in the request from the client. */
        status = read(data.sfd, &buffer, MAXDATASIZE);
        if(status < 0)
        {
            cerr << "Error receiving data from the client.\n";
            continue;
        }

        /* Send the response to the client. */
        status = write(data.sfd, buffer_str.c_str(), buffer_str.length());
        if(status < 0)
        {
            cerr << "Error writing data to client.\n";
        }
    }

    /* Close the sockets and exits. */
    close(data.sfd);
    close(data.lfd);
    return 0;
}