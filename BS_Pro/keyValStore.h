
#ifndef UNTITLED_KEYVALSTORE_H
#define UNTITLED_KEYVALSTORE_H
#define MAX_STRING_LENGTH 128

typedef struct {
    char command[MAX_STRING_LENGTH];
    char key[MAX_STRING_LENGTH];
    char value[MAX_STRING_LENGTH];
} UserInput;

typedef struct {
  int code;
  char message[MAX_STRING_LENGTH];
} OperationResult;

OperationResult put(char*, char*);
OperationResult get(char*, char*);
OperationResult del(char*);
int isValidKeyOrValue(char*);
OperationResult validateUserInput(UserInput userInput);
UserInput stringToUserInput(char*);

#endif //UNTITLED_KEYVALSTORE_H
