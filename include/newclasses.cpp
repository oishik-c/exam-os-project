#include "newclasses.h"

// Function to encrypt a string using a Caesar cipher with a specified shift value
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

// Function to signal the end of a client-server connection
void endconnection(int clientSocket)
{
    int code = END_CONNECTION;
    send(clientSocket, &code, sizeof(code), 0);
}

// Implementation of the Question class using Constructor
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

// Function to print a question
string Question::printQuestion()
{
    return this->text + "\n1)" + this->options[0] + "\n2)" + this->options[1] + "\n3)" + this->options[2] + "\n4)" + this->options[3];
}

// Implementation of the User class using constructor
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

// Implementation of the Student class using constructor
Student::Student(const string &u, const string &p, const string &id) : User(u, p), studentId(id) {}

string Student::getStudentId()
{
    return this->studentId;
}

string Student::getUserType()
{
    return "Student";
}

// Function to start an exam for a student
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
        system("clear"); // Assuming a Unix-based system to clear the terminal
        cout << textToRead->buffer << endl
             << "A: ";
        cin >> answer;
        send(clientSocket, &answer, sizeof(answer), 0);
    }
    if (examend)
    {
        cout << "Exam has ended!" << endl;
    }
    usleep(5000); // Pause for a short time to allow the server to process the results
    recv(clientSocket, &score, sizeof(score), 0);
    cout << "The score obtained: " << score << endl;
}

// Implementation of the Teacher class using constructor
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
    // Function to send a set of questions to the client via a socket
    ifstream questionFile(filepath, ios::binary);
    textsendtype *textToSend = new textsendtype;
    while (!questionFile.eof())
    {
        questionFile.read(textToSend->buffer, sizeof(textToSend->buffer));
        textToSend->bytesRead = questionFile.gcount();
        textToSend->code = SET_Q_REQUEST;
        send(clientSocket, textToSend, sizeof(*textToSend), 0);
        cout << textToSend->buffer << endl;
        sleep(1);
    }
    textToSend->code = Q_END_SIG;
    send(clientSocket, textToSend, sizeof(*textToSend), 0);
    questionFile.close();
}

// Function to check if a username is unique in the registered users file
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
                    return false; // Username already exists
                }
            }
        }
        registerFileReader.close();
    }
    return true; // Username is unique
}

// Function to register a user
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
            break; // Username is unique; continue with the registration process
        }

        else
        {
            code = NO_SGNL;
            send(clientSocket, &code, sizeof(code), 0);
            continue; // Prompt the client to choose a different username
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
        // Append user information to the registered users file
        registerFileWriter << uname << "|" << hashedPassword << "|" << newUserInfo->type << "|" << newUserInfo->id << "\n";
        registerFileWriter.close();
        code = RGSTR_SCCSFL;
    }
    else
    {
        perror("FILENOTOPEN");
        code = NO_SGNL; // Registration failed due to a file error
    }
    cout << hashedPassword << uname << endl;
    send(clientSocket, &code, sizeof(code), 0);
}

// Function to register a user with helper functions
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
            break; // Username is unique; continue with the registration process
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
        return user; // User registration is successful
    }
    else
    {
        cout << "Registration Failed! Server Error!" << endl;
        return NULL; // User registration failed due to a server error
    }
}

// Function to handle user login
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
                            // Compare the login credentials with stored credentials.
                            if (newUserInfo->username == storedUsername && generatedPassword == storedPassword && newUserInfo->type == storedUserType)
                                code = LGN_SCCSFL; // Login successful
                            else
                                code = LGN_FAIL; // Login failed due to incorrect credentials
                        }
                    }
                }
            }
        }
        registerFileReader.close();
    }
    else
        code = NO_SGNL; // Login failed due to a file error

    // Send the result code back to the client.
    send(clientSocket, &code, sizeof(code), 0);
}

// This function assists in the login process by interacting with the user.
User *loginHelper(int &clientSocket, string &usertype)
{
    int code;
    userinfosendtype *newUserInfo = new userinfosendtype;

    // Prompt the user to enter their username and password.
    cout << "Enter your username: ";
    cin >> newUserInfo->username;
    cout << "Enter your password: ";
    cin >> newUserInfo->password;
    // Set the user type (Student or Teacher) for the login request.
    strcpy(newUserInfo->type, usertype.c_str());
    // Send the user's login information to the server.
    send(clientSocket, newUserInfo, sizeof(*newUserInfo), 0);
    // Receive the login result code from the server.
    recv(clientSocket, &code, sizeof(code), 0);

    // Handle different cases based on the received code.
    switch (code)
    {
    case LGN_SCCSFL:
    {
        User *user;
        cout << "Welcome, " << newUserInfo->username << "!!" << endl;
        // Create a User object based on the user type (Student or Teacher).
        if (strcmp(newUserInfo->type, "S") == 0)
            user = new Student(newUserInfo->username, newUserInfo->password, newUserInfo->id);
        else
            user = new Teacher(newUserInfo->username, newUserInfo->password, newUserInfo->id);
        return user; // User login is successful
    }
    case LGN_FAIL:
    {
        cerr << "Invalid Username or Password! Please retry login!";
        return NULL; // User login failed due to incorrect credentials
    }
    case NO_SGNL:
    {
        cerr << "Login Failed!! Server Error!!" << endl;
        return NULL; // User login failed due to a server error
    }
    }
    return NULL;
}

// This is the constructor for the Client class.
Client::Client()
{
    // Create a socket to establish a connection with the server.
    this->clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (this->clientSocket == -1)
    {
        perror("Creation of Client Socket: ");
        exit(1);
    }

    string ipAddr;
    cout << "Enter the IP address of server: ";
    cin >> ipAddr;

    // Initialize the server's address structure.
    this->serverAddr.sin_addr.s_addr = inet_addr(ipAddr.c_str());
    this->serverAddr.sin_port = htons(12345);
    this->serverAddr.sin_family = AF_INET;
    this->serverAddrLen = sizeof(serverAddr);

    // Connect to the server using the established socket.
    if (connect(clientSocket, (struct sockaddr *)&this->serverAddr, serverAddrLen) == -1)
    {
        perror("Connection Error: ");
        exit(1);
    }

    cout << "Server Connected Successfully!!\n"
         << endl;

    int choice;
    system("clear"); // Clear the terminal screen
    cout << register_menu << "Choice: ";
    cin >> choice;
    switch (choice)
    {
    case 1:
    {
        // Send a registration request to the server and prompt the user to choose a user type (Student or Teacher).
        int code = RGSTR_REQ;
        send(clientSocket, &code, sizeof(code), 0);
        string userType;
        cout << "Are you a student or a teacher?(S for Student, T for Teacher) ";
        cin >> userType;
        // Call the registerUserHelper function to assist in the registration process.
        this->user = registerUserHelper(this->clientSocket, userType);
        break;
    }
    case 2:
    {
        // Send a login request to the server and prompt the user to choose a user type (Student or Teacher).
        int code = LGN_REQ;
        send(clientSocket, &code, sizeof(code), 0);
        string userType;
        cout << "Are you a student or a teacher?(S for Student, T for Teacher) ";
        cin >> userType;
        // Call the loginHelper function to assist in the login process.
        this->user = loginHelper(this->clientSocket, userType);
        break;
    }
    case 3:
    {
        cout << "Exiting!!" << endl;

        // Send an end connection request to the server and close the client socket.
        endconnection(this->clientSocket);
        close(this->clientSocket);
        exit(0);
    }
    }
}

// This function handles different requests and options for the client.
int Client::requests()
{
    // Check if the user is of type Student.
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
                // Start an exam for the student.
                student->startExam(this->clientSocket);
                break;
            }
            case 2:
            {
                // Send an end connection request to the server and close the client socket to exit.
                endconnection(this->clientSocket);
                close(this->clientSocket);
                exit(0);
            }
            }
        }
    }
    // Check if the user is of type Teacher.
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
                // Send a request to set questions and prompt the user for a text file.
                int code = SET_Q_REQUEST;
                send(this->clientSocket, &code, sizeof(code), 0);
                string filePath;
                cout << "Enter path to text file: ";
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                getline(cin, filePath);

                // Call the teacher's function to set questions using the provided text file.
                teacher->setQuestions(this->clientSocket, filePath);
                cout << "DONE" << endl;
                recv(this->clientSocket, &code, sizeof(code), 0);

                // Check if the questions were set successfully and provide feedback.
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
                // Send a request to see questions and receive and display questions from the server.
                int code = SEE_Q_REQUEST;
                send(this->clientSocket, &code, sizeof(code), 0);
                char buffer[1024];
                int len;
                int bytesRead;
                bool questend = false;
                while (true)
                {
                    // Receive and display questions until the "EOE" (End of Exam) signal is received.
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
                // Send an end connection request to the server and close the client socket to exit.
                endconnection(this->clientSocket);
                close(this->clientSocket);
                exit(0);
            }
            }
        }
    }
    return 0;
}

// Destructor for the Client class.
Client::~Client()
{
    // Send an end connection request to the server and close the client socket. Free memory used by the user object.
    endconnection(this->clientSocket);
    close(this->clientSocket);
    free(user);
}