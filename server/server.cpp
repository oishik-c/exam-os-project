#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <pthread.h>
#include "definitions.h"
#include "newclasses.h"

using namespace std;

int serverSocket;
struct sockaddr_in serverAddr;
socklen_t serverAddrLen = sizeof(serverAddr);
list<pthread_t> childThreads;
vector<Question> question_bank;

int giveExam(int clientSocket)
{
    /* Function to handle the examination of a client
    input : clientSocket*/
    pthread_t ptid = pthread_self();
    int answer, score = 0;
    textsendtype *textToSend = new textsendtype;
    for (int i = 0; i < question_bank.size(); i++)
    {
        strcpy(textToSend->buffer, question_bank[i].printQuestion().c_str());
        textToSend->bytesRead = question_bank[i].printQuestion().length();
        textToSend->buffer[textToSend->bytesRead] = '\0';
        if (send(clientSocket, textToSend, sizeof(*textToSend), 0) <= 0)
        {
            perror("Transmission Error");
            pthread_exit(&ptid);
        }
        if (recv(clientSocket, &answer, sizeof(answer), 0) <= 0)
        {
            perror("Receiving Error");
            pthread_exit(&ptid);
        }
        cout << answer << endl;
        if (answer == question_bank[i].getCorrectOption())
        {
            score++;
        }
    }
    strcpy(textToSend->buffer, "EOE");
    textToSend->buffer[3] = '\0';
    send(clientSocket, textToSend, sizeof(*textToSend), 0);
    return score;
}

vector<Question> &parseQuestionFile(const string &file_path)
{
    question_bank.clear();
    ifstream file(file_path);

    if (!file.is_open())
    {
        cerr << "Failed to open the file: " << file_path << endl;
        return question_bank;
    }

    string line;
    while (getline(file, line))
    {
        istringstream ss(line);
        string text, options[4];
        int correct;
        string part;

        for (int i = 0; i < 6; ++i)
        {
            if (!getline(ss, part, '|'))
            {
                cerr << "Invalid line: " << line << endl;
                break;
            }

            if (i == 0)
            {
                text = part;
            }
            else if (i == 5)
            {
                correct = stoi(part);
            }
            else
            {
                options[i - 1] = part;
            }
        }

        Question *question = new Question(text, options, correct);
        question_bank.push_back(*question);
    }

    file.close();
    return question_bank;
}

void *handleClient(void *arg)
{
    pthread_t ptid = pthread_self();
    int clientSocket = *((int *)arg), code;
    bool endflag = false;
    while (true)
    {
        recv(clientSocket, &code, sizeof(code), 0);
        cout << code << endl;

        switch (code)
        {
        case END_CONNECTION:
        {
            endflag = true;
            break;
        }
        case EXAM_START_REQUEST:
        {
            int score = giveExam(clientSocket);
            if (send(clientSocket, &score, sizeof(score), 0) <= 0)
            {
                perror("Score Not Sending: ");
                pthread_exit(&ptid);
            }
            break;
        }
        case SET_Q_REQUEST:
        {
            ofstream outFile(questionFilePath, ios::binary);
            textsendtype *newtext = new textsendtype;
            while (true)
            {
                recv(clientSocket, newtext, sizeof(*newtext), 0);
                if (newtext->code != Q_END_SIG)
                {
                    outFile.write(newtext->buffer, newtext->bytesRead);
                    cout << newtext->buffer << endl;
                }
                else
                    break;
            }
            cout << "END" << endl;
            outFile.close();
            question_bank.clear();
            question_bank = parseQuestionFile(questionFilePath);
            code = SET_Q_ACK;
            send(clientSocket, &code, sizeof(code), 0);
            break;
        }
        case SEE_Q_REQUEST:
        {
            pthread_t ptid = pthread_self();
            int len;
            for (int i = 0; i < question_bank.size(); i++)
            {
                string text = question_bank[i].printQuestion();
                len = text.length();
                if (send(clientSocket, &len, sizeof(len), 0) <= 0)
                {
                    perror("Transmission Error: ");
                    pthread_exit(&ptid);
                }
                if (send(clientSocket, text.c_str(), text.length(), 0) <= 0)
                {
                    perror("Transmission Error: ");
                    pthread_exit(&ptid);
                }
            }
            send(clientSocket, &len, sizeof(len), 0);
            send(clientSocket, "EOE", 4, 0);
            break;
        }
        case RGSTR_REQ:
        {
            registerUser(clientSocket);
            break;
        }
        case LGN_REQ:
            login(clientSocket);
            break;
        }
        if (endflag)
            break;
    }
    childThreads.remove(pthread_self());
    close(clientSocket);
    return NULL;
}

void *handleConnections(void *arg)
{
    // Handling connections
    while (true)
    {
        int clientSocket;
        if ((clientSocket = accept(serverSocket, (sockaddr *)&serverAddr, &serverAddrLen)) == -1)
        {
            perror("Accepting");
            exit(1);
        }
        pthread_t temp;
        pthread_create(&temp, NULL, handleClient, &clientSocket);
        childThreads.insert(childThreads.begin(), temp);
        pthread_detach(temp);
    }
    return NULL;
}

int main()
{
    // Creating the server socket for connections
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        perror("Socket creation: ");
        exit(1);
    }

    // Binding the server socket to the server address
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_family = AF_INET;

    if (bind(serverSocket, (const sockaddr *)&serverAddr, (socklen_t)serverAddrLen) == -1)
    {
        perror("Bind: ");
        exit(1);
    }

    // Listening to the port
    if (listen(serverSocket, 5) == -1)
    {
        perror("Listen: ");
        exit(1);
    }

    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        return -1;
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr != nullptr && ifa->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(addr->sin_addr), ip, INET_ADDRSTRLEN);
            cout << ifa->ifa_name << ": " << ip << endl;
        }
    }

    freeifaddrs(ifaddr);
    cout << "Server listening on port 12345..." << endl;

    pthread_t clienthandler;
    if (pthread_create(&clienthandler, NULL, handleConnections, NULL) == -1)
    {
        perror("Client Handler Error:");
        exit(1);
    }
    pthread_detach(clienthandler);

    int choice;
    while (true)
    {
        cout << "Enter operation:\n1)Q SET\n2)Display question_bank\n3)Exit" << endl;
        cin >> choice;
        switch (choice)
        {
        case 1:
        {
            string qfpath;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Enter path to text file: ";
            getline(cin, qfpath);
            question_bank = parseQuestionFile(qfpath);
            break;
        }
        case 2:
        {
            for (auto question : question_bank)
            {
                string q = question.printQuestion();
                cout << q << endl;
            }
            break;
        }
        case 3:
        {
            close(serverSocket);
            exit(0);
        }
        default:
            break;
        }
    }

    close(serverSocket);
    return 0;
}