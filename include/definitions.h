// Define directory paths for client and communication data
#define clientDirectory "/home/jraj/OS Project/clients/"
#define communicationDirectory "/home/jraj/OS Project/communication/"

// Define file paths for user registration and temporary question storage
#define registerFilePath "../include/register/registeredUsers.txt"
#define questionFilePath "../temp.txt"
#define lederboardFilePath "../include/register/leaderboard.txt"

// Custom code for seeking the beginning of a file
#define seek_comm_front                \
    communicationFile.seekg(ios::beg); \
    communicationFile.seekp(ios::beg);

// Define unique signal codes for communication
#define SHM_CREATED 10000000
#define NEXT_QUESTION_REQUEST 11000000
#define QUESTION_SENT 7000000
#define EVALUATE_ANSWER 800000
#define QUESTION_END 9000000

// Signals used for socket communication
#define NO_SGNL -1
#define EXAM_START_REQUEST 101
#define SET_Q_REQUEST 201
#define SET_Q_ACK 202
#define Q_END_SIG 203
#define SEE_Q_REQUEST 204
#define Q_RECV_SCCS 205
#define USER_UNQ 001
#define RGSTR_REQ 002
#define RGSTR_SCCSFL 003
#define LGN_REQ 004
#define LGN_SCCSFL 005
#define LGN_FAIL 006
#define REQ_STATS 301
#define REQ_LDRBRD 302
#define END_CONNECTION INT8_MIN

// Menu options for user interaction
#define register_menu "Enter your choice:\n1)Register\n2)Login\n3)Exit\n"
#define student_menu "Enter your choice:\n1)Start Exam\n2)Check Leaderboard\n3)Exit\n"
#define teacher_menu "Enter your choice:\n1)Set Questions\n2)See Questions\n3)Check Leaderboard\n4)See Student Statistic\n5)Exit\n"