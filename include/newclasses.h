#ifndef NEWCLASSES_H
#define NEWCLASSES_H

#include <bits/stdc++.h> // Include standard C++ library headers
#include <fstream> // Include file stream for file operations
#include <arpa/inet.h>// Include definitions for internet operations
#include <netinet/in.h>// Include internet socket address structure
#include <unistd.h>// Include POSIX operating system API
#include <stdlib.h>// Include standard library definitions
#include <cstring>// Include C-style string manipulation functions
#include <cstdlib>// Include C standard library functions
#include <semaphore.h>// Include POSIX semaphore support
#include <fcntl.h>// Include file control options
#include "definitions.h"// Include user-defined definitions
using namespace std;    // Use the standard C++ namespace


// Structure to hold data for sending text messages
struct textsendtype
{
    
    char buffer[2048];// Buffer to store text data
    int code;// Code to identify the message type
    int bytesRead;// Number of bytes read
};

// Structure to hold user information for registration and login
struct userinfosendtype
{
    char username[64];// User's username
    char password[30];// User's password
    char id[30];// User's ID
    char type[4];// User type (e.g., Student or Teacher)
};

// Class to represent a question
class Question
{
private:
    string text;// Text of the question
    string options[4];// Array to store options for the question
    int correctOption;// Index of the correct option

public:
    // Constructor to initialize a Question object
    Question(string &text, string *options, int &correct);

    // Setter method to set the text of the question
    void setText(string &text);

    // Getter method to get the text of the question
    string getText();

    // Setter method to set an option for the question
    void setOption(string &option, int &i);

    // Getter method to get an option of the question
    string getOption(int &i);

    // Setter method to set the correct answer for the question
    void setCorrectAnswer(int &correct);

    // Getter method to get the index of the correct option
    int getCorrectOption();

    // Method to create a formatted string of the question
    string printQuestion();
};

// Base class to represent a user
class User
{
private:
    string username;// User's username
    string password;// User's password

public:
    // Constructor to initialize a User object
    User(const string &u, const string &p);

    // Getter method to get the username
    string getUsername();

    // Getter method to get the password
    string getPassword();

    // Virtual method to get the user type (subclasses will override this)
    virtual string getUserType();
};

// Derived class representing a Student, inheriting from User
class Student : public User
{
private:
    string studentId;// Student's ID

public:
    // Constructor to initialize a Student object
    Student(const string &u, const string &p, const string &id);

    // Getter method to get the student's ID
    string getStudentId();

    // Override the base class method to get the user type
    string getUserType();

    // Method to start an exam for the student
    void startExam(int clientSocket);
};

// Derived class representing a Teacher, inheriting from User
class Teacher : public User
{
private:
    string teacherId;// Teacher's ID

public:
    // Constructor to initialize a Teacher object
    Teacher(const string &u, const string &p, const string &id);
    // Getter method to get the teacher's ID
    string getTeacherId();

    // Override the base class method to get the user type
    string getUserType();

    // Method to set questions for the teacher
    void setQuestions(int &clientSocket, string &filepath);
};

// Class to represent a client
class Client
{
private:
    int clientSocket;// Client socket descriptor
    User *user;// Pointer to a User object
    struct sockaddr_in serverAddr;// Server address structure
    socklen_t serverAddrLen;// Length of the server address structure


public:
    // Constructor to create a client
    Client();
    // Destructor to clean up client resources
    ~Client();
    // Method to handle client requests
    int requests();
};

// Function to check if a username is unique
bool isUsernameUnique(const string &username);

// Helper function to register a user
User *registerUserHelper(int &clientSocket, string &userType);


// Function to register a user
void registerUser(int &clientSocket);

// Function to log in a user
void login(int &clientSocket);

// Helper function to log in a user
User *loginHelper(int &clientSocket, string &usertype);

// Function to end the client-server connection
void endconnection(int clientSocket);

#endif // NEWCLASSES_H