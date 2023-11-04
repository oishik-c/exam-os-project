#define clientDirectory "/home/oishik/OS Project/clients/"
#define communicationDirectory "/home/oishik/OS Project/communication/"
#define registerFilePath "register/registeredUsers.txt"
#define questionFilePath "question_bank/temp.txt"

#define seek_comm_front                \
    communicationFile.seekg(ios::beg); \
    communicationFile.seekp(ios::beg);

#define SHM_CREATED 10000000
#define NEXT_QUESTION_REQUEST 11000000
#define QUESTION_SENT 7000000
#define EVALUATE_ANSWER 800000
#define QUESTION_END 9000000

// Signals for sockets
#define NO_SGNL -1
#define EXAM_START_REQUEST 101
#define SET_Q_REQUEST 201
#define SET_Q_ACK 202
#define Q_END_SIG 203
#define SEE_Q_REQUEST 204
#define USER_UNQ 001
#define RGSTR_REQ 002
#define RGSTR_SCCSFL 003
#define LGN_REQ 004
#define LGN_SCCSFL 005
#define LGN_FAIL 006
#define END_CONNECTION INT8_MIN

#define register_menu "Enter your choice:\n1)Register\n2)Login\n3)Exit\n"
#define student_menu "Enter your choice:\n1)Start Exam\n2)Exit\n"
#define teacher_menu "Enter your choice:\n1)Set Questions\n2)See Questions\n3)Exit\n"