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

// Create a semaphore for file access synchronization
sem_t *regFileSemaphore = sem_open("/semaphore-rf", O_CREAT, S_IRUSR | S_IWUSR, 0);

// Declare server-related variables
int serverSocket;
struct sockaddr_in serverAddr;
socklen_t serverAddrLen = sizeof(serverAddr);
list<pthread_t> childThreads;
vector<Question> questionBank;

// Function to handle the examination for a client
int giveExam(int clientSocket)
{
    pthread_t ptid = pthread_self();
    int answer, score = 0;
    textsendtype *textToSend = new textsendtype;
    for (int i = 0; i < questionBank.size(); i++)
    {
        // Prepare and send the question to the client
        strcpy(textToSend->buffer, questionBank[i].printQuestion().c_str());
        textToSend->bytesRead = questionBank[i].printQuestion().length();
        textToSend->buffer[textToSend->bytesRead] = '\0';
        if (send(clientSocket, textToSend, sizeof(*textToSend), 0) <= 0)
        {
            perror("Transmission Error");
            pthread_exit(&ptid);
        }
        // Receive the client's answer
        if (recv(clientSocket, &answer, sizeof(answer), 0) <= 0)
        {
            perror("Receiving Error");
            pthread_exit(&ptid);
        }
        cout << answer << endl;
        // Check the answer and update the score
        if (answer == questionBank[i].getCorrectOption())
        {
            score++;
        }
    }
    // Signal the end of the examination to the client
    strcpy(textToSend->buffer, "EOE");
    textToSend->buffer[3] = '\0';
    send(clientSocket, textToSend, sizeof(*textToSend), 0);
    return score;
}

// Function to parse the question file and store the question in question bank
vector<Question> &parseQuestionFile(const string &file_path)
{
    questionBank.clear();
    ifstream file(file_path);

    if (!file.is_open())
    {
        cerr << "Failed to open the file: " << file_path << endl;
        return questionBank;
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

        // Create a Question object and add it to the question bank
        Question *question = new Question(text, options, correct);
        questionBank.push_back(*question);
    }

    file.close();
    return questionBank;
}


// Function to handle a client's requests
void *handleClient(void *arg)
{
    pthread_t ptid = pthread_self();
    int clientSocket = *((int *)arg), code;
    bool endflag = false;
    while (true)
    {
        // Receive a request code from the client
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
            // Handle the examination request and send the score back to the client
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
            // Receive and store questions from the client
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

            // Update the question bank with new questions
            outFile.close();
            questionBank.clear();
            questionBank = parseQuestionFile(questionFilePath);
            remove(questionFilePath);
            code = SET_Q_ACK;
            send(clientSocket, &code, sizeof(code), 0);
            break;
        }
        case SEE_Q_REQUEST:
        {
            pthread_t ptid = pthread_self();
            int len;
            for (int i = 0; i < questionBank.size(); i++)
            {
                
                string text = questionBank[i].printQuestion();
                len = text.length();

                // Send questions to the client
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
            // Handle user registration request
            sem_post(regFileSemaphore);
            registerUser(clientSocket);
            sem_wait(regFileSemaphore);
            break;
        }
        case LGN_REQ:
        {
            // Handle user login request
            sem_post(regFileSemaphore);
            login(clientSocket);
            sem_wait(regFileSemaphore);
            break;
        }
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
    // Continuously handle incoming client connections
    while (true)
    {
        int clientSocket;
        if ((clientSocket = accept(serverSocket, (sockaddr *)&serverAddr, &serverAddrLen)) == -1)
        {
            perror("Accepting");
            exit(1);
        }
        // Create a new thread to handle the client's requests
        pthread_t temp;
        pthread_create(&temp, NULL, handleClient, &clientSocket);
        // Add the new thread to the list of child threads
        childThreads.insert(childThreads.begin(), temp);
        // Detach the child thread to allow it to run independently
        pthread_detach(temp);
    }
    return NULL;
}

int main()
{
    // Create the server socket for handling client connections
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

    // Retrieve and display available IP addresses of the server
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
            cout << "IP Address: " << ip << endl;
        }
    }
    freeifaddrs(ifaddr);

    cout << "Server listening on port 12345..." << endl;

    // Create a thread to handle incoming client connections
    pthread_t clienthandler;
    if (pthread_create(&clienthandler, NULL, handleConnections, NULL) == -1)
    {
        perror("Client Handler Error:");
        exit(1);
    }

    // Detach the client handler thread to allow it to run independently
    pthread_detach(clienthandler);

    int choice;
    while (true)
    {
        cout << "Enter operation:\n1)Q SET\n2)Display questionBank\n3)Exit" << endl;
        cin >> choice;
        switch (choice)
        {
        case 1:
        {
            string qfpath;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Enter path to text file: ";
            // Read the path to the question file and update the question bank
            getline(cin, qfpath);
            questionBank = parseQuestionFile(qfpath);
            break;
        }
        case 2:
        {
            // Display the questions in the question bank
            for (auto question : questionBank)
            {
                string q = question.printQuestion();
                cout << q << endl;
            }
            break;
        }
        case 3:
        {
            // Close the server socket and exit the program
            close(serverSocket);
            exit(0);
        }
        default:
            break;
        }
    }

    // Close the server socket
    close(serverSocket);
    return 0;
}