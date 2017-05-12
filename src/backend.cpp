/*
	Sabin Raj Tiwari
	CMSC 621
	Project 3
*/

#include <arpa/inet.h>
#include <fstream>
#include <vector>

/* Define the global constants. */
#define MAXDATASIZE 1024

/* Import namespaces. */
using namespace std;

/* Structures for the program. */
struct record
{
	int id;
    double balance;
};

/* Global variables. */
std::fstream log_file;
std::vector<record> records;

/* Gets the record with the provided id if it exists. */
record get_record(int id)
{
    for(int i = 0; i < records.size(); i++)
    {
        if(records.at(i).id == id) {
            return record;
        }
    }
}

/*  Main method that handles the client program logic. */
int main(int argc, char **argv)
{
    
}