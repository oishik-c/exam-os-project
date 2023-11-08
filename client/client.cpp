#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include "definitions.h"
#include "newclasses.h"
using namespace std;

int main()
{
    // Create a client object to handle user interactions and requests
    Client client;
    // Start the client's request-handling loop
    client.requests();
    return 0;
}