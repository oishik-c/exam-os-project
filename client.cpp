#include <bits/stdc++.h>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "definitions.h"
#include "newclasses.h"
using namespace std;

/*
void exam(int clientSocket)
{
    int code = EXAM_START_REQUEST, score;
    send(clientSocket, &code, sizeof(code), 0);
    char buffer[1024];
    int len;
    bool examend = false;
    int bytesRead;
    while (true)
    {
        if (recv(clientSocket, &len, sizeof(len), 0) <= 0)
        {
            perror("Length Receive Error: ");
            exit(1);
        }
        if ((bytesRead = recv(clientSocket, buffer, len, 0)) <= 0)
        {
            perror("Question Receive Error: ");
            exit(1);
        }
        buffer[bytesRead] = '\0';
        int answer;
        if (strcmp(buffer, "EOE") == 0)
        {
            examend = true;
            break;
        }
        cout << buffer << "A: ";
        cin >> answer;
        send(clientSocket, &answer, sizeof(answer), 0);
    }
    if (examend)
    {
        cout << "Exam has ended!" << endl;
    }
    usleep(5000);
    recv(clientSocket, &score, sizeof(score), 0);
    cout << "The score obtained: " << score << endl;
}

void endconnection(int clientSocket)
{
    int code = END_CONNECTION;
    send(clientSocket, &code, sizeof(code), 0);
}

int main()
{
    int clientSocket;
    struct sockaddr_in serverAddr;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        cerr << "Error creating client socket." << endl;
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(12345);

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        cerr << "Error connecting to the server." << endl;
        return 1;
    }

    int choice;
    while (true)
    {
        cout << "Enter choice:\n1)Take Exam\n2)Exit" << endl;
        cin >> choice;
        switch (choice)
        {
        case 1:
        {
            exam(clientSocket);
            break;
        }
        case 2:
        {
            endconnection(clientSocket);
            break;
        }
        }
        if (choice == 2)
            break;
    }
    close(clientSocket);

    return 0;
}
*/

int main()
{
    Client client;
    client.requests();
    return 0;
}