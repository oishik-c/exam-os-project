#define clientDirectory "/home/oishik/OS Project/clients/"
#define communicationDirectory "/home/oishik/OS Project/communication/"
#define registerFilePath "register/registeredUsers.txt"
#define questionFilePath "question_bank/temp.txt"

#define seek_comm_front                \
    communicationFile.seekg(ios::beg); \
    communicationFile.seekp(ios::beg);

#define SHM_CREATED 5
#define NEXT_QUESTION_REQUEST 6
#define QUESTION_SENT 7
#define EVALUATE_ANSWER 8
#define QUESTION_END 9

// Signals for sockets
#define EXAM_START_REQUEST 0
#define SET_Q_REQUEST 1
#define SET_Q_ACK 2
#define Q_END_SIG 3
#define SEE_Q_REQUEST 4
#define END_CONNECTION INT8_MIN

#define register_menu "Enter your choice:\n1)Register\n2)Login\n3)Exit\n"
#define student_menu "Enter your choice:\n1)Start Exam\n2)Exit\n"
#define teacher_menu "Enter your choice:\n1)Set Questions\n2)See Questions\n3)Exit\n"