#ifndef MESSAGES_H
#define MESSAGES_H

#define NUMBEROFMSGS 64

#define STUDENT_MESSAGES_FILE "messagesSTD.txt"
#define TEACHER_MESSAGES_FILE "messagesTCH.txt"

typedef struct {
  int number;
  char format[120];
} DEBUG_MESSAGES;

extern DEBUG_MESSAGES DebugMessages[NUMBEROFMSGS];

int Messages_Get_Pos(int number);
int Messages_Load_Messages(int, char *);

#endif

