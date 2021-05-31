
#ifndef UNTITLED_KEYVALSTORE_H
#define UNTITLED_KEYVALSTORE_H
#define MAX_STRING_LENGTH 128

typedef struct {
    char command[MAX_STRING_LENGTH];
    char key[MAX_STRING_LENGTH];
    char value[MAX_STRING_LENGTH];
} UserInput;

int put(char*, char*);
int get(char*, char*);
int del(char*);
int isValidKeyOrValue(char*);
int isValidUserInput(UserInput);
UserInput stringToUserInput(char*);

#endif //UNTITLED_KEYVALSTORE_H
