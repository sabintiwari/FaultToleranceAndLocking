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
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

/* Define the global constants. */
#define MAXDATASIZE 1024

/* Import namespaces. */
using namespace std;

/* Structures for the program. */
struct socket_data
{
	int fd, port;
    std::string host;
	struct sockaddr_in address;
};

/* Get the string value from an int. */
std::string itos(int value)
{
	std::stringstream str;
	str << value;
	return str.str();
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

/* Checks if the string value is a currency. */
bool is_currency(string value)
{
    char *d;
    strtod(value.c_str(), &d);
    if(*d) return false;
    vector<string> tokens = split(value, '.');
    if(tokens.size() != 2 || tokens[1].length() != 2) return false;
    return true;
}

/* Checks if the string value is an int. */
bool is_int(string value)
{
    char *d;
    strtol(value.c_str(), &d, 10);
    return *d == 0;
}

/* Establishes a connection to the server. */
bool connect(socket_data data)
{
    if(connect(data.fd, (struct sockaddr *)&(data.address), sizeof(data.address)) < 0)
    {
        cerr << "Failed to connect to the server.\n";
        return false;
    }
    return true;
}

/* Performs one cycle of send and receive with the server. */
void send(socket_data data, string message)
{
    /* Connect to the server. */
    if(connect(data))
    {
        /* Send a message to the server to perform the transaction. */
        char buffer[MAXDATASIZE];
        int status = write(data.fd, message.c_str(), message.size());
        if(status < 0)
        {
            cerr << "Failed to send message to server.\n";
            close(data.fd);
            return;
        }

        /* Wait for the response to be received from the server. */
        memset(&buffer[0], 0, MAXDATASIZE);
        status = read(data.fd, &buffer, MAXDATASIZE);
        if(status < 0)
        {
            cerr << "Failed to receive message from server.\n";
            close(data.fd);
            return;
        }

        /* Show the message from the server. */
        cout << buffer << "\n";
    }
}

/* Handles the create functionality. */
void create(socket_data data, vector<string> tokens)
{
    /* Validate inputs for the create. */
    if(tokens.size() < 2 || !is_currency(tokens[1])) 
        cerr << "Use create with a money input: CREATE <initial_amount>\n";
    else 
        send(data, "CREATE:" + tokens[1]);
}

/* Handles the update functionality. */
void update(socket_data data, vector<string> tokens)
{
    /* Validate inputs for the update. */
    if(tokens.size() < 3 || !is_int(tokens[1]) || !is_currency(tokens[2])) 
        cerr << "Use update with account and money input: UPDATE <acct_id> <value>\n";
    else
        send(data, "UPDATE:" + tokens[1] + ":" + tokens[2]);
}

/* Handles the query functionality. */
void query(socket_data data, vector<string> tokens)
{
    /* Validate inputs for the query. */
    if(tokens.size() < 2 || !is_int(tokens[1]))
        cerr << "Use query with account: QUERY <acct_id>\n";
    else
        send(data, "QUERY:" + tokens[1]);
}

/*  Main method that handles the client program logic. */
int main(int argc, char **argv)
{
    if(argc < 3)
    {
        /* Show error if the correct number of argumnets were not passed. */
        cerr << "Usage: client <server_address> <port>\n";
        exit(1);
    }

    /* Declare the socket data. */
	socket_data data;
    data.host = argv[1];
    data.port = atoi(argv[2]);

    /* Setup the connection information to the front-end server. */
    struct hostent *server;
    server = gethostbyname(data.host.c_str());
    if(server == NULL)
    {
        /* Show error if the server does not exist. */
        cerr << "No host exists with the address: " << data.host << "\n";
        exit(1);
    }

    /* Get the address tot the front-end server. */
    data.address.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&data.address.sin_addr.s_addr, server->h_length);
    data.address.sin_port = htons(data.port);

    /* Setup the socket. */
    data.fd = socket(AF_INET, SOCK_STREAM, 0);
    if(data.fd < 0) 
    {
        /* Show error if the socket creation fails. */
        cerr << "Socket is not formed.\n";
        exit(1);
    }

    /* Variables for the loop. */
    bool end = false;
    string input;
    vector<string> tokens;
    cout << "Welcome to Bank Service! Please enter your command.\n";

    /* Loop that handles the UI. */
    while (!end)
    {
        cout << "> ";
        getline(cin, input);

        /* Tokenize the received input. */
        std::transform(input.begin(), input.end(), input.begin(), ::toupper);
        tokens = split(input, ' ');
        if(tokens.size() == 0) continue;

        /* Check the query and perform the respective function. */
        if(tokens[0] == "CREATE")
            create(data, tokens);
        else if(tokens[0] == "UPDATE")
            update(data, tokens);
        else if(tokens[0] == "QUERY")
            query(data, tokens);
        else if(tokens[0] == "QUIT")
            end = true;
        else 
        {
            cout << "Error! Invalid query entered.\n";
            cout << "Valid options are: CREATE, UPDATE, QUERY, or QUIT.\n";
        }
    }

    /* Close the connection before exiting. */
    cout << "Exiting program.";
    close(data.fd);
    return 0;
}