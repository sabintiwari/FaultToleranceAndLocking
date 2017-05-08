/*
	Sabin Raj Tiwari
	CMSC 621
	Project 3
*/

/* Include header files. */
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
#define MAXCONNECTIONS 16
#define MAXDATASIZE 1024
#define MAXTHREADS 16

/* Import namespaces. */
using namespace std;

/* Structures for the program. */
struct socket_data
{
	int lfd, sfd, port;
    std::string host;
	struct sockaddr_in address, client;
};

/* Global variables. */
int used_threads = 0;
pthread_t threads[MAXTHREADS];

/* Get the string value from an int. */
std::string itos(int value)
{
	std::stringstream str;
	str << value;
	return str.str();
}

/* Splits a string using a delimeter. */
template<typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

/* Uses split() to get the vector of string elements. */
std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

/* Thread to handle th client request. */
void *client_request(void *args)
{
    /* Get the socket data from the args. */
    socket_data data = *((socket_data*)args);

    /* Values to store the responses. */
    char buffer[MAXDATASIZE];
    string buffer_str, client_addr;

    /* Read in the request from the client. */
    client_addr = inet_ntoa(data.client.sin_addr);
    int s = read(data.sfd, &buffer, MAXDATASIZE);
    if(s < 0)
    {
        cerr << "Error receiving dat afrom the client.\n";
    }
    else
    {
        /* Get the data from the buffer and store it in the string. */
        buffer[s] = '\0';
        buffer_str = buffer;

        /* Split the token from the client and perform the transaction. */
        vector<string> tokens = split(buffer_str, ':');
        /* Check the query and perform the respective function. */
        if(tokens[0] == "CREATE")
            create(data, tokens);
        else if(tokens[0] == "UPDATE")
            update(data, tokens);
        else if(tokens[0] == "QUERY")
            query(data, tokens);

        /* Send the response to the client. */
        s = write(data.sfd, buffer_str.c_str(), buffer_str.length());
        if(s < 0)
        {
            cerr << "Error writing data to client.\n";
        }
    }

    /* Close the socket once te request is complete. */
    close(data.sfd);
    used_threads--;
    cout << "Used threads (-): " + itos(used_threads) + "\n";
}

/* Main function login for the communicator program. */
int main(int argc, char * argv[])
{
    if(argc < 2)
    {
        /* Show error if the correct number of arguments were not passed. */
		cerr << "Usage: comm <port_number>\n";
		exit(1);
    }

    /* Create the stream socket for the communicator. */
    socket_data data;
    data.lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(data.lfd < 0)
    {
        cerr << "Error creating socket.\n";
        exit(1);
    }

    /* Declare the socket data and communicator address. */
    data.port = atoi(argv[1]);
    data.address.sin_family = AF_INET;
    data.address.sin_addr.s_addr = INADDR_ANY;
    data.address.sin_port = htons(data.port);

    /* Bind the communicator address to the listen file descriptor. */
    if(bind(data.lfd, (struct sockaddr *)&(data.address), sizeof(data.address)) < 0)
    {
        cerr << "Error binding socket.\n";
        exit(1);
    }

    /* Bind the communicator address to the listen file descriptor. */
    listen(data.lfd, MAXCONNECTIONS);
    while(1)
    {
        /* Accept a client request. */
        socklen_t addrlen = sizeof(data.client);
        data.sfd = accept(data.lfd, (struct sockaddr *)&(data.client), &addrlen);
        if(data.sfd < 0)
        {
            cerr << "Error accepting client request.\n";
            exit(1);
        }

        /* Create a thread call the client request function. */
        pthread_create(&threads[used_threads], NULL, client_request, &data);
        used_threads++;
        cout << "Used threads (+): " + itos(used_threads) + "\n";
    }

    return 0;
}