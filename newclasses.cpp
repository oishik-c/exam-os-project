#include "newclasses.h"

void endconnection(int clientSocket)
{
    int code = END_CONNECTION;
    send(clientSocket, &code, sizeof(code), 0);
}

Question::Question(string &text, string *options, int &correct)
{
    this->text = text;
    for (int i = 0; i < 4; i++)
    {
        this->options[i] = options[i];
    }
    this->correctOption = correct;
}

void Question::setText(string &text)
{
    this->text = text;
}

string Question::getText()
{
    return this->text;
}

void Question::setOption(string &option, int &i)
{
    this->options[i] = option;
}

string Question::getOption(int &i)
{
    return this->options[i];
}

void Question::setCorrectAnswer(int &correct)
{
    this->correctOption = correct;
}

int Question::getCorrectOption()
{
    return this->correctOption;
}

string Question::printQuestion()
{
    return this->text + "\n1)" + this->options[0] + "\n2)" + this->options[1] + "\n3)" + this->options[2] + "\n4)" + this->options[3];
}

User::User(const string &u, const string &p) : username(u), password(p) {}

string User::getUsername()
{
    return this->username;
}

string User::getPassword()
{
    return this->password;
}

string User::getUserType()
{
    return "User";
}

Student::Student(const string &u, const string &p, const string &id) : User(u, p), studentId(id) {}

string Student::getStudentId()
{
    return this->studentId;
}

string Student::getUserType()
{
    return "Student";
}

void Student::startExam(int clientSocket)
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
        cout << buffer << endl
             << "A: ";
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

Teacher::Teacher(const string &u, const string &p, const string &id) : User(u, p), teacherId(id) {}

string Teacher::getTeacherId()
{
    return this->teacherId;
}

string Teacher::getUserType()
{
    return "Teacher";
}

void Teacher::setQuestions(int &clientSocket, string &filepath)
{
    ifstream questionFile(filepath, ios::binary);
    char buffer[1024];
    int bytesRead, code = SET_Q_REQUEST;
    while (!questionFile.eof())
    {
        send(clientSocket, &code, sizeof(code), 0);
        questionFile.read(buffer, sizeof(buffer));
        bytesRead = questionFile.gcount();
        send(clientSocket, buffer, bytesRead, 0);
        cout << buffer << endl;
    }
    questionFile.close();
}

bool isUsernameUnique(const string &username)
{
    ifstream registerFileReader(registerFilePath);
    if (registerFileReader.is_open())
    {
        string line;
        while (getline(registerFileReader, line))
        {
            size_t pos = line.find('|');
            if (pos != string::npos)
            {
                string storedUsername = line.substr(0, pos);
                if (username == storedUsername)
                {
                    registerFileReader.close();
                    return false;
                }
            }
        }
        registerFileReader.close();
    }
    return true;
}

User *registerUser(string &userType)
{
    string username, password, id;
    while (true)
    {
        cout << "Enter your username: ";
        cin >> username;
        if (isUsernameUnique(username))
        {
            break;
        }
        cout << "Username already exists. Please choose a different username." << endl;
    }
    cout << "Enter your password: ";
    cin >> password;
    cout << "Enter your id: ";
    cin >> id;

    ofstream registerFileWriter(registerFilePath, ios::app);
    if (registerFileWriter.is_open())
    {
        User *user;
        registerFileWriter << username << "|" << password << "|" << userType << "|" << id << "\n";
        registerFileWriter.close();
        cout << "Registration successful" << endl;
        if (userType == "S")
            user = new Student(username, password, id);
        else
            user = new Teacher(username, password, id);
        cout << "Welcome, " << username << "!!\n";
        return user;
    }
    else
    {
        cerr << "Error opening the registration file." << endl;
        return NULL;
    }
}

User *login(string &usertype)
{
    string username, password;
    cout << "Enter your username: ";
    cin >> username;
    cout << "Enter your password: ";
    cin >> password;

    ifstream registerFileReader(registerFilePath);
    if (registerFileReader.is_open())
    {
        string line;
        while (getline(registerFileReader, line))
        {
            size_t pos = line.find('|');
            if (pos != string::npos)
            {
                string storedUsername = line.substr(0, pos);
                string storedPasswordType = line.substr(pos + 1);
                size_t pos2 = storedPasswordType.find('|');
                if (pos2 != string::npos)
                {
                    string storedPassword = storedPasswordType.substr(0, pos2);
                    string userTypeId = storedPasswordType.substr(pos2 + 1);
                    size_t pos3 = userTypeId.find('|');
                    if (pos3 != string::npos)
                    {
                        string userType = userTypeId.substr(0, pos3);
                        string id = userTypeId.substr(pos3 + 1);
                        if (username == storedUsername && password == storedPassword && usertype == userType)
                        {
                            User *user;
                            cout << "Welcome, " << username << "!!" << endl;
                            if (userType == "S")
                                user = new Student(username, password, id);
                            else
                                user = new Teacher(username, password, id);
                            registerFileReader.close();
                            return user;
                        }
                    }
                }
            }
        }
        registerFileReader.close();
    }
    else
    {
        cerr << "Error opening the registration file." << endl;
        return NULL;
    }

    cerr << "Invalid username or password." << endl;
    return NULL;
}

Client::Client()
{
    this->clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (this->clientSocket == -1)
    {
        perror("Creation of Client Socket: ");
        exit(1);
    }

    string ipAddr;
    cout << "Enter the IP address of server: ";
    cin >> ipAddr;

    this->serverAddr.sin_addr.s_addr = inet_addr(ipAddr.c_str());
    this->serverAddr.sin_port = htons(12345);
    this->serverAddr.sin_family = AF_INET;
    this->serverAddrLen = sizeof(serverAddr);

    if (connect(clientSocket, (struct sockaddr *)&this->serverAddr, serverAddrLen) == -1)
    {
        perror("Connection Error: ");
        exit(1);
    }

    cout << "Server Connected Successfully!!\n"
         << endl;

    int choice;
    cout << register_menu << "Choice: ";
    cin >> choice;
    switch (choice)
    {
    case 1:
    {
        string userType;
        cout << "Are you a student or a teacher?(S for Student, T for Teacher) ";
        cin >> userType;
        this->user = registerUser(userType);
        break;
    }
    case 2:
    {
        string userType;
        cout << "Are you a student or a teacher?(S for Student, T for Teacher) ";
        cin >> userType;
        this->user = login(userType);
        break;
    }
    case 3:
    {
        cout << "Exiting!!" << endl;
        endconnection(this->clientSocket);
        close(this->clientSocket);
        exit(0);
    }
    }
}

int Client::requests()
{
    if (Student *student = dynamic_cast<Student *>(this->user))
    {
        int choice;
        while (true)
        {
            cout << student_menu << "Choice: ";
            cin >> choice;
            switch (choice)
            {
            case 1:
            {
                student->startExam(this->clientSocket);
                break;
            }
            case 2:
            {
                endconnection(this->clientSocket);
                close(this->clientSocket);
                exit(0);
            }
            }
        }
    }
    if (Teacher *teacher = dynamic_cast<Teacher *>(this->user))
    {
        int choice;
        while (true)
        {
            cout << teacher_menu << "Choice: ";
            cin >> choice;
            switch (choice)
            {
            case 1:
            {
                int code = SET_Q_REQUEST;
                send(this->clientSocket, &code, sizeof(code), 0);
                string filePath;
                cout << "Enter path to text file: ";
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                getline(cin, filePath);
                teacher->setQuestions(this->clientSocket, filePath);
                sleep(3);
                code = Q_END_SIG;
                send(this->clientSocket, &code, sizeof(code), 0);
                cout << "DONE" << endl;
                recv(this->clientSocket, &code, sizeof(code), 0);
                if (code == SET_Q_ACK)
                {
                    cout << "Questions set successfully!!" << endl;
                }
                else
                {
                    cerr << "Questions set failed!!" << endl;
                }
                break;
            }
            case 2:
            {
                int code = SEE_Q_REQUEST;
                send(this->clientSocket, &code, sizeof(code), 0);
                char buffer[1024];
                int len;
                int bytesRead;
                bool questend = false;
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
                    if (strcmp(buffer, "EOE") == 0)
                    {
                        questend = true;
                        break;
                    }
                    cout << buffer << endl;
                }
                if (questend)
                {
                    cout << "No more questions!" << endl;
                }
                break;
            }
            case 3:
            {
                endconnection(this->clientSocket);
                close(this->clientSocket);
                exit(0);
            }
            }
        }
    }
    return 0;
}

Client::~Client()
{
    endconnection(this->clientSocket);
    close(this->clientSocket);
    free(user);
}