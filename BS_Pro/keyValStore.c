//
// Created by fatma on 08.05.2021.
//

#include <stdio.h>
#include <string.h>
#include "keyValStore.h"

// Adjusts the paths to the OS.
#ifdef _WIN32
    #define DATA_FILE_PATH "..\\storage\\data.txt"
    #define TEMP_FILE_PATH "..\\storage\\temp.txt"
    #define STORAGE_PATH "..\\storage\\"
#else
    #define DATA_FILE_PATH "../BS_Pro/storage/data.txt"
    #define TEMP_FILE_PATH "../BS_Pro/storage/temp.txt"
    #define STORAGE_PATH "../BS_Pro/storage/"
#endif

#define KEY_VALUE_FORMAT_STRING "%s => %s\n"
#define FILE_FORMAT ".txt"
#define MAX_STRING_LENGTH 200


// Should be used in put, get and del.
int isValidKeyOrValue(char *candidate) {
    while (*candidate != '\0') {
        if (!(*candidate >= 48 && *candidate <= 57)          // is not a number
            && !(*candidate >= 65 && *candidate <= 90)       // and is not an uppercase letter
            && !(*candidate >= 97 && *candidate <= 122)) {   // and is not a lowercase letter
            return 0; // -> is not valid
        }
        candidate++;
    }
    return 1; // -> is valid
}

// Only needed for the first option.
void createDataFileIfNonExistent() {
    FILE *fp = fopen(DATA_FILE_PATH, "r");
    if (fp == NULL) {
        fp = fopen(DATA_FILE_PATH, "w");
        fprintf(fp, KEY_VALUE_FORMAT_STRING, "*clientKey*", "*clientValue*");
        fprintf(stderr, "Could not find the data file, so a new one was created.\n");
    }
    fclose(fp);
}

// Only needed for the second option.
void buildFilePath(char *destination, char *fileName) {
    strcat(destination, STORAGE_PATH);
    strcat(destination, fileName);
    strcat(destination, FILE_FORMAT);
}

/*
 * First option: One data file to rule them all.
 * Requires to copy the whole file to a new one on every put() or del().
 * Actually overwriting whole lines without a temp file is more complicated.
 * https://www.quora.com/How-do-I-delete-the-first-line-of-a-text-file-without-creating-another-text-file-in-C?share=1
 */
int put(char *clientKey, char *clientValue) {
    int valueWasOverwritten = 0;
    char currentKey[MAX_STRING_LENGTH] = "";
    char currentValue[MAX_STRING_LENGTH] = "";

    createDataFileIfNonExistent();
    FILE *sourceFile = fopen(DATA_FILE_PATH, "r");
    FILE *destinationFile = fopen(TEMP_FILE_PATH, "w");

    // Copy every line to the new file and exchange currentValue for clientValue if the clientKey already exists.
    while (!feof(sourceFile)) {
        fscanf(sourceFile, KEY_VALUE_FORMAT_STRING, currentKey, currentValue);
        if (valueWasOverwritten == 0 && strcmp(currentKey, clientKey) == 0) {
            fprintf(destinationFile, KEY_VALUE_FORMAT_STRING, clientKey, clientValue);
            valueWasOverwritten = 1;
        } else {
            fprintf(destinationFile, KEY_VALUE_FORMAT_STRING, currentKey, currentValue);
        }
    }

    // Adds the key-value pair to the end of the file if the clientKey did not already exist.
    if (valueWasOverwritten == 0) {
        fprintf(destinationFile, KEY_VALUE_FORMAT_STRING, clientKey, clientValue);
    }

    // Close files and exchange the data file for the temp file.
    fclose(sourceFile);
    fclose(destinationFile);
    remove(DATA_FILE_PATH);
    rename(TEMP_FILE_PATH, DATA_FILE_PATH);

    // 0 -> A new key-value pair was added.
    // 1 -> The old key-value pair was overwritten.
    return valueWasOverwritten;
}

/*
 * Second option: One file per key.
 * Simpler functions for put(), get() and del().
 * Probably faster but results in a lot of files.
 */
int putAlt(char *clientKey, char *clientValue) {
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
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

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

int del(char *value) {
    return 0;
}
