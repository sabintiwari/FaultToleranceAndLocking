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
	int id;
    double balance;
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
    return data;
}

/* Global variables. */
std::fstream log_file;
std::vector<record> records;
int total_accounts = 100;
const int oneresp = 1;
const int zeroresp = 0;

/* Get the string value from an int. */
std::string itos(int value)
{
	std::stringstream str;
	str << value;
	return str.str();
}

/* Get the string value for money. */
std::string mtos(double value)
{
	std::ostringstream moneystream;
	moneystream << fixed << std::setprecision(2) << value;
	return moneystream.str();
}

/* Get the string value from an double. */
std::string dtos(double value)
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
    while (std::getline(ss, item, delim)) 
    {
        *(result++) = item;
    }
}

/* Uses split() to get the vector of string elements. */
std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

/* Gets the record with the provided id if it exists. */
int get_record(int id)
{
    for(int i = 0; i < records.size(); i++)
    {
        if(records.at(i).id == id) 
        {
            return i;
        }
    }
    return -1;
}

/* Updates the record at the provided index. */
double update_record(int index, double amount)
{
    records.at(index).balance += amount;
    return records.at(index).balance;
}

/* Handle the commit request from the coordinator. */
void commit_abort(int commit, int fd, string account, string amount)
{
    int acct = atoi(account.c_str());
    double amt = atof(amount.c_str());
    double response = -1;

    /* If the vote was commit, perform the transaction. */
    if(commit == 1)
    {
        if(acct < 0)
        {
            /* Create account */
            total_accounts++;
            response = total_accounts;
            records.push_back(construct_record(total_accounts, amt));
        }
        else
        {
            int i = get_record(acct);
            if(i > -1)
            {
                if(amt < 0)
                {
                    /* Query account */
                    response = records.at(i).balance;
                }
                else
                {
                    /* Update account */
                    response = update_record(i, amt);
                }
            }
        }
    }

    int status = write(fd, &response, sizeof(double));
    if(status < 0)
    {
        cerr << "Failed to write data to the front-end server.\n";
    }
}

/* Sends the vote and waits for the result. */
int send_vote(int response, int fd)
{
    int status = write(fd, &response, sizeof(int));
    if(status < 0)
    {
        cerr << "Failed to write data to the front-end server.\n";
    }

    status = read(fd, &response, sizeof(int));
    if(status < 0)
    {
        cerr << "Failed to read data from the front-end server.\n";
    }

    return response;
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
    int id = getpid();
    records.push_back(construct_record(-1, -1.00));
    cout << "Back-end server [ID: " << id << "] is waiting for front-end server requests...\n";
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
    int status, response;

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

        /* Check the buffer and split it into tokens */
        buffer[status] = '\0';
		buffer_str = buffer;
        vector<string> tokens = split(buffer_str, ':');

        /* Check the transaction type. */
        if(tokens[0] == "CREATE")
        {
             response = 1;
             response = send_vote(response, data.sfd);
             commit_abort(response, data.sfd, "-1", tokens[1]);
        }
        else if(tokens[0] == "UPDATE")
        {
            int i = get_record(atoi(tokens[1].c_str()));
            if(i > -1) response = 1;
            else response = 0;
            response = send_vote(response, data.sfd);
            commit_abort(response, data.sfd, tokens[1], tokens[2]);
        }
        else if(tokens[0] == "QUERY")
        {
            int i = get_record(atoi(tokens[1].c_str()));
            if(i > -1) response = 1;
            else response = 0;
            response = send_vote(response, data.sfd);
            commit_abort(response, data.sfd, tokens[1], "-1");
        }
    }

    /* Close the sockets and exits. */
    close(data.sfd);
    close(data.lfd);
    return 0;
}