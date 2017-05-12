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
int used_threads = 0, messages = 0;
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

/* Performs the function of creating an account. */
string create(socket_data data, vector<string> tokens)
{
	
	return "OK";
}

/* Performs the function of updating an account. */
string update(socket_data data, vector<string> tokens)
{
	return "OK";
}

/* Performs the function of querying an account. */
string query(socket_data data, vector<string> tokens)
{
	return "OK";
}

/* Thread to handle th client request. */
void *client_request(void *args)
{
    /* Get the socket data from the args. */
    socket_data data = *((socket_data*)args);

    /* Values to store the responses. */
    char buffer[MAXDATASIZE];
    string buffer_str, client_addr;
    bool end = false;
    int status;

    /* Loop that handles the client request. */
    while(!end)
    {
        /* Read in the request from the client. */
        client_addr = inet_ntoa(data.client.sin_addr);
        status = read(data.sfd, &buffer, MAXDATASIZE);
        if(status < 0)
        {
            cerr << "Error receiving dat afrom the client.\n";
            continue;
        }

        /* Get the data from the buffer and store it in the string. */
        buffer[status] = '\0';
        buffer_str = buffer;
        cout << "Received data from the client: [" << buffer_str << "]\n";
		messages++;

        /* Split the token from the client and perform the transaction. */
        vector<string> tokens = split(buffer_str, ':');
        /* Check the query and perform the respective function. */
        if(tokens[0] == "CREATE") 
			buffer_str = create(data, tokens);
        else if(tokens[0] == "UPDATE") 
			buffer_str = update(data, tokens);
        else if(tokens[0] == "QUERY") 
			buffer_str = query(data, tokens);
        else
		{
			buffer_str = "OK";
			end = true;
		}
            
        /* Send the response to the client. */
        status = write(data.sfd, buffer_str.c_str(), buffer_str.length());
        if(status < 0)
        {
            cerr << "Error writing data to client.\n";
        }
    }

    close(data.sfd);
    used_threads--;
    cout << "Used threads (-): " + itos(used_threads) + "\n";
}

/* Main function login for the server program. */
int main(int argc, char **argv)
{
    if(argc < 2)
    {
        /* Show error if the correct number of arguments were not passed. */
		cerr << "Usage: comm <port_number>\n";
		exit(1);
    }

    /* Create the stream socket for the server. */
    socket_data data;
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

    /* Bind the server address to the listen file descriptor. */
    cout << "Server waiting for client requests...\n";
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