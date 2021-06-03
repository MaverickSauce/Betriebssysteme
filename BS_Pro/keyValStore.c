
#include <stdio.h>
#include <string.h>
#include "keyValStore.h"

#define STORAGE_PATH "../BS_Pro/storage/"
#define FILE_FORMAT ".txt"


// access and modify stored data

void buildFilePath(char *destination, char *fileName) {
    strcat(destination, STORAGE_PATH);
    strcat(destination, fileName);
    strcat(destination, FILE_FORMAT);
}

int put(char *clientKey, char *clientValue) {
    int fileWasOverwritten = 0;
    char filePath[MAX_STRING_LENGTH] = "";

    buildFilePath(filePath, clientKey);

    // On Unix this could be replaced with access() but unisted.h does not work on Windows.
    FILE *targetFile = fopen(filePath, "r");
    if (targetFile != NULL) {
        fclose(targetFile);
        fileWasOverwritten = 1;
    }
    targetFile = fopen(filePath, "w");
    fprintf(targetFile, "%s", clientValue);
    fclose(targetFile);

    // 0 -> A new file was added.
    // 1 -> The old file was overwritten.
    return fileWasOverwritten;
}

int get(char *key, char *res) {
    char filePath[MAX_STRING_LENGTH] = "";
    buildFilePath(filePath, key);

    FILE *targetFile = fopen(filePath, "r");
    if(targetFile){
        fgets(res, MAX_STRING_LENGTH, (FILE *) targetFile);
        fclose(targetFile);
        return 0;
    } else {
        return -2;
    }
}

int del(char *key) {
    char filePath[MAX_STRING_LENGTH] = "";
    buildFilePath(filePath, key);

    FILE* fp;
    fp = fopen(filePath, "r");
    if (fp == NULL) {
        return -2;      // -2 -> key not found
    } else {
        fclose(fp);
        return remove(filePath); // -1 -> an error occured
    }                            // 0 -> key-value pair was deleted succesfully
}


// parsing and validation of input

int isValidKeyOrValue(char *candidate) {
    while (*candidate != '\0') {
        if (!(*candidate >= 48 && *candidate <= 57)          // is not a number
            && !(*candidate >= 65 && *candidate <= 90)       // and is not an uppercase letter
            && !(*candidate >= 97 && *candidate <= 122)) {   // and is not a lowercase letter
            return 0;                                        // -> is not valid
        }
        candidate++;
    }
    return 1;  // -> is valid
}

int isValidSystemOperation(char *candidate) {
    if (strcmp(candidate, "date") != 0
        && strcmp(candidate, "uptime") != 0
        && strcmp(candidate, "Who") != 0) {
        return 0; // not valid
    }
    return 1; // -> is valid
}

int strcpyNoNewLine(char dest[], const char source[]) {
    int position = 0;
    while (source[position] != '\0' && source[position] != '\r' & source[position] != '\n') {
        dest[position] = source[position];
        position++;
    }
    dest[position] = '\0';

    return 0;
}

UserInput stringToUserInput(char *rawString) {
    UserInput userInput;
    char str[MAX_STRING_LENGTH];
    int i = 0;
    char *seperator = " ";
    strcpyNoNewLine(str, rawString);
    puts(str);

    char *token;

    token = strtok(str, seperator);      // Set the first token

    while (token != NULL) {             // Copy into userInput and walk through the remaining tokens
        if (i == 0) {
            strcpy(userInput.command, token);
            i++;
        } else if (i == 1) {
            strcpy(userInput.key, token);
            i++;
        } else {
            strcpy(userInput.value, token);
            break;
        }
        token = strtok(NULL, seperator);
    }
    return userInput;
}

/*  1 -> valid
 * -1 -> invalid command
 * -2 -> invalid arguments
 * -3 -> too many arguments
 */
int isValidUserInput(UserInput userInput) {
    if (strcmp(userInput.command, "PUT") == 0) {
        if (isValidKeyOrValue(userInput.key) && (isValidKeyOrValue(userInput.value))) {
            return 1; // valid
        } else {
            return -2; // invalid key or value
        }
    } else if (strcmp(userInput.command, "GET") == 0
                || strcmp(userInput.command, "DEL") == 0
                || strcmp(userInput.command, "SUB") == 0) {
        if (isValidKeyOrValue(userInput.key)) {
            if (strcmp(userInput.value, "") == 0) {
                return 1; // valid
            } else {
                return -3; // too many arguments
            }
        } else {
            return -2; // invalid key
        }
    } else if (strcmp(userInput.command, "QUIT") == 0
               || strcmp(userInput.command, "BEG") == 0
               || strcmp(userInput.command, "END") == 0) {
        if (strcmp(userInput.key, "") == 0 || strcmp(userInput.value, "") == 0) {
            return 1; // valid
        } else {
            return -3; // too many arguments
        }
    } else if (strcmp(userInput.command, "OP") == 0) {
        if (isValidKeyOrValue(userInput.key)) {
            if (isValidSystemOperation(userInput.value)) {
                return 1; // valid
            } else {
                return -2; // invalid system operation
            }
        } else {
            return -2; // invalid key
        }
    } else {
        return -1; // invalid command
    }
}
