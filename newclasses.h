#ifndef NEWCLASSES_H
#define NEWCLASSES_H

#include <bits/stdc++.h>
#include <fstream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <cstdlib>
#include "definitions.h"
using namespace std;

class Question
{
private:
    string text;
    string options[4];
    int correctOption;

public:
    Question(string &text, string *options, int &correct);
    void setText(string &text);
    string getText();
    void setOption(string &option, int &i);
    string getOption(int &i);
    void setCorrectAnswer(int &correct);
    int getCorrectOption();
    string printQuestion();
};

class User
{
private:
    string username;
    string password;

public:
    User(const string &u, const string &p);
    string getUsername();
    string getPassword();
    virtual string getUserType();
};

class Student : public User
{
private:
    string studentId;

public:
    Student(const string &u, const string &p, const string &id);
    string getStudentId();
    string getUserType();
    void startExam(int clientSocket);
};

class Teacher : public User
{
private:
    string teacherId;

public:
    Teacher(const string &u, const string &p, const string &id);
    string getTeacherId();
    string getUserType();
    void setQuestions(int &clientSocket, string &filepath);
};

class Client
{
private:
    int clientSocket;
    User *user;
    struct sockaddr_in serverAddr;
    socklen_t serverAddrLen;

public:
    Client();
    ~Client();
    int requests();
};

bool isUsernameUnique(const string &username);

User *registerUser(string &userType);

User *login(string &usertype);

void endconnection(int clientSocket);

#endif // NEWCLASSES_H