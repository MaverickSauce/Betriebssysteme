
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

OperationResult put(char *clientKey, char *clientValue) {
    OperationResult result;
    memset(result.message, '\0', sizeof(result.message));

    char filePath[MAX_STRING_LENGTH] = "";
    buildFilePath(filePath, clientKey);

    FILE *targetFile = fopen(filePath, "r");
    if (targetFile != NULL) {
        fclose(targetFile);
        result.code = -1;
        strcpy(result.message, ":key_overwritten");
    } else {
        result.code = 0;
        strcpy(result.message, ":new_key_added");
    }
    targetFile = fopen(filePath, "w");
    fprintf(targetFile, "%s", clientValue);
    fclose(targetFile);

    //  0 -> A new key was added.
    // -1 -> The old key was overwritten.
    return result;
}

OperationResult get(char *key, char *res) {
    OperationResult result;
    memset(result.message, '\0', sizeof(result.message));

    char filePath[MAX_STRING_LENGTH] = "";
    buildFilePath(filePath, key);

    FILE *targetFile = fopen(filePath, "r");
    if(targetFile){
        fgets(res, MAX_STRING_LENGTH, (FILE *) targetFile);
        fclose(targetFile);
        result.code = 0;
        strcpy(result.message, "key_found");
    } else {
        result.code = -1;
        strcpy(result.message, "key_nonexistent");
    }

    //  0 -> key was found
    // -1 -> key does not exist
    return result;
}

OperationResult del(char *key) {
    OperationResult result;
    memset(result.message, '\0', sizeof(result.message));

    char filePath[MAX_STRING_LENGTH] = "";
    buildFilePath(filePath, key);

    FILE* fp = fopen(filePath, "r");
    if (fp == NULL) {
        result.code = -2;
        strcpy(result.message, "key_nonexistent");
    } else {
        fclose(fp);
        result.code = remove(filePath);
        if (result.code == -1) {
            strcpy(result.message, "unexpected_error");
        } else {
            strcpy(result.message, "key_deleted");
        }
    }

    //  0 -> key-value pair was deleted succesfully
    // -1 -> an error occured
    // -2 -> key not found
    return result;
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

void strcpyNoNewLine(char dest[], const char source[]) {
    int position = 0;
    while (source[position] != '\0' && source[position] != '\r' & source[position] != '\n') {
        dest[position] = source[position];
        position++;
    }
    dest[position] = '\0';
}

UserInput stringToUserInput(char *rawString) {
    UserInput userInput;

    // initialize userInput with empty strings
    memset(userInput.command, '\0', sizeof(userInput.command));
    memset(userInput.key, '\0', sizeof(userInput.key));
    memset(userInput.value, '\0', sizeof(userInput.value));

    char str[MAX_STRING_LENGTH];
    int i = 0;
    char *separator = " ";
    char *token;

    strcpyNoNewLine(str, rawString);
    token = strtok(str, separator);      // Set the first token

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
        token = strtok(NULL, separator);
    }
    return userInput;
}

OperationResult validateUserInput(UserInput userInput) {
    OperationResult result;
    memset(result.message, '\0', sizeof(result.message));

    // validate userInput and fill in code
    if (strcmp(userInput.command, "PUT") == 0) {
        if (isValidKeyOrValue(userInput.key) && (isValidKeyOrValue(userInput.value))) {
            result.code = 0; // valid
        } else {
            result.code = -2; // invalid key or value
        }
    } else if (strcmp(userInput.command, "GET") == 0
                || strcmp(userInput.command, "DEL") == 0
                || strcmp(userInput.command, "SUB") == 0) {
        if (isValidKeyOrValue(userInput.key)) {
            if (strcmp(userInput.value, "") == 0) {
                result.code = 0; // valid
            } else {
                result.code = -3; // too many arguments
            }
        } else {
            result.code = -2; // invalid key
        }
    } else if (strcmp(userInput.command, "QUIT") == 0
               || strcmp(userInput.command, "BEG") == 0
               || strcmp(userInput.command, "END") == 0) {
        if (strcmp(userInput.key, "") == 0 || strcmp(userInput.value, "") == 0) {
            result.code = 0; // valid
        } else {
            result.code = -3; // too many arguments
        }
    } else if (strcmp(userInput.command, "OP") == 0) {
        if (isValidKeyOrValue(userInput.key)) {
            if (isValidSystemOperation(userInput.value)) {
                result.code = 0; // valid
            } else {
                result.code = -2; // invalid system operation
            }
        } else {
            result.code = -2; // invalid key
        }
    } else {
        result.code = -1; // invalid command
    }

    // fill in message
    switch (result.code) {
        case 0:
            strcpy(result.message, "valid_input");
            break;
        case -1:
            strcpy(result.message, "check_command");
            break;
        case -2:
            strcpy(result.message, "check_arguments");
            break;
        case -3:
            strcpy(result.message, "too_many_arguments");
            break;
        default:
            strcpy(result.message, "unexpected_validation_result");
            break;
    }

    return result;
}
