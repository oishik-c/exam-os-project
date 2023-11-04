#include "newclasses.h"

string encryptString(const string &input, int shift)
{
    string encrypted = input;

    for (char &c : encrypted)
    {
        if (isalpha(c))
        {
            char base = islower(c) ? 'a' : 'A';
            c = static_cast<char>(base + (c - base + shift) % 26);
        }
    }

    return encrypted;
}

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
    bool examend = false;
    while (true)
    {
        textsendtype *textToRead = new textsendtype;
        if (recv(clientSocket, textToRead, sizeof(*textToRead), 0) <= 0)
        {
            perror("Question Receiving Error");
            exit(1);
        }
        int answer;
        if (strcmp(textToRead->buffer, "EOE") == 0)
        {
            examend = true;
            break;
        }
        system("clear");
        cout << textToRead->buffer << endl
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
    textsendtype *textToSend = new textsendtype;
    while (!questionFile.eof())
    {
        questionFile.read(textToSend->buffer, sizeof(textToSend->buffer));
        textToSend->bytesRead = questionFile.gcount();
        textToSend->code = SET_Q_REQUEST;
        send(clientSocket, textToSend, sizeof(*textToSend), 0);
        cout << textToSend->buffer << endl;
    }
    textToSend->code = Q_END_SIG;
    send(clientSocket, textToSend, sizeof(*textToSend), 0);
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

void registerUser(int &clientSocket)
{
    int code, key;
    string uname, pword, uid, utype, hashedPassword;
    userinfosendtype *newUserInfo = new userinfosendtype;
    while (true)
    {
        recv(clientSocket, newUserInfo, sizeof(*newUserInfo), 0);
        string username = newUserInfo->username;
        if (isUsernameUnique(username))
        {
            code = USER_UNQ;
            send(clientSocket, &code, sizeof(code), 0);
            break;
        }
        else
        {
            code = NO_SGNL;
            send(clientSocket, &code, sizeof(code), 0);
            continue;
        }
    }

    recv(clientSocket, newUserInfo, sizeof(*newUserInfo), 0);
    uname = newUserInfo->username;
    pword = newUserInfo->password;
    uid = newUserInfo->id;
    utype = newUserInfo->type;
    key = 13;
    hashedPassword = encryptString(pword, key);
    ofstream registerFileWriter(registerFilePath, ios::app);
    if (registerFileWriter.is_open())
    {
        registerFileWriter << uname << "|" << hashedPassword << "|" << newUserInfo->type << "|" << newUserInfo->id << "\n";
        registerFileWriter.close();
        code = RGSTR_SCCSFL;
    }
    else
    {
        perror("FILENOTOPEN");
        code = NO_SGNL;
    }
    cout << hashedPassword << uname << endl;
    send(clientSocket, &code, sizeof(code), 0);
}

User *registerUserHelper(int &clientSocket, string &userType)
{
    int code;
    userinfosendtype *newUserInfo = new userinfosendtype;
    while (true)
    {
        cout << "Enter your username: ";
        cin >> newUserInfo->username;
        send(clientSocket, newUserInfo, sizeof(*newUserInfo), 0);
        recv(clientSocket, &code, sizeof(code), 0);
        if (code != USER_UNQ)
            cout << "Username already exists. Please choose a different username." << endl;
        else
            break;
    }
    cout << "Enter your password: ";
    cin >> newUserInfo->password;
    cout << "Enter your id: ";
    cin >> newUserInfo->id;
    strcpy(newUserInfo->type, userType.c_str());

    send(clientSocket, newUserInfo, sizeof(*newUserInfo), 0);
    recv(clientSocket, &code, sizeof(code), 0);

    User *user;
    if (code == RGSTR_SCCSFL)
    {
        if (userType == "S")
            user = new Student(newUserInfo->username, newUserInfo->password, newUserInfo->id);
        else
            user = new Teacher(newUserInfo->username, newUserInfo->password, newUserInfo->id);
        cout << "Welcome, " << newUserInfo->username << "!!\n";
        return user;
    }
    else
    {
        cout << "Registration Failed! Server Error!" << endl;
        return NULL;
    }
}

void login(int &clientSocket)
{
    int code;
    userinfosendtype *newUserInfo = new userinfosendtype;
    recv(clientSocket, newUserInfo, sizeof(*newUserInfo), 0);

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
                        string storedUserType = userTypeId.substr(0, pos3);
                        string storedIdSalt = userTypeId.substr(pos3 + 1);
                        size_t pos4 = storedIdSalt.find('|');
                        if (pos4 != string::npos)
                        {
                            string storedId = storedIdSalt.substr(0, pos4);
                            int key = 13;
                            string generatedPassword = encryptString(newUserInfo->password, key);
                            if (newUserInfo->username == storedUsername && generatedPassword == storedPassword && newUserInfo->type == storedUserType)
                                code = LGN_SCCSFL;
                            else
                                code = LGN_FAIL;
                        }
                    }
                }
            }
        }
        registerFileReader.close();
    }
    else
        code = NO_SGNL;

    send(clientSocket, &code, sizeof(code), 0);
}

User *loginHelper(int &clientSocket, string &usertype)
{
    int code;
    userinfosendtype *newUserInfo = new userinfosendtype;
    cout << "Enter your username: ";
    cin >> newUserInfo->username;
    cout << "Enter your password: ";
    cin >> newUserInfo->password;
    strcpy(newUserInfo->type, usertype.c_str());
    send(clientSocket, newUserInfo, sizeof(*newUserInfo), 0);
    recv(clientSocket, &code, sizeof(code), 0);
    switch (code)
    {
    case LGN_SCCSFL:
    {
        User *user;
        cout << "Welcome, " << newUserInfo->username << "!!" << endl;
        if (strcmp(newUserInfo->type, "S") == 0)
            user = new Student(newUserInfo->username, newUserInfo->password, newUserInfo->id);
        else
            user = new Teacher(newUserInfo->username, newUserInfo->password, newUserInfo->id);
        return user;
    }
    case LGN_FAIL:
    {
        cerr << "Invalid Username or Password! Please retry login!";
        return NULL;
    }
    case NO_SGNL:
    {
        cerr << "Login Failed!! Server Error!!" << endl;
        return NULL;
    }
    }
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
    system("clear");
    cout << register_menu << "Choice: ";
    cin >> choice;
    switch (choice)
    {
    case 1:
    {
        int code = RGSTR_REQ;
        send(clientSocket, &code, sizeof(code), 0);
        string userType;
        cout << "Are you a student or a teacher?(S for Student, T for Teacher) ";
        cin >> userType;
        this->user = registerUserHelper(this->clientSocket, userType);
        break;
    }
    case 2:
    {
        int code = LGN_REQ;
        send(clientSocket, &code, sizeof(code), 0);
        string userType;
        cout << "Are you a student or a teacher?(S for Student, T for Teacher) ";
        cin >> userType;
        this->user = loginHelper(this->clientSocket, userType);
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