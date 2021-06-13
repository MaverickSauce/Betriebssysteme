
#ifndef BETRIEBSSYSTEME_SUBSCRIPTION_H
#define BETRIEBSSYSTEME_SUBSCRIPTION_H
#include "keyValStore.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

typedef struct {
    int subscriberPID;
    char key[MAX_STRING_LENGTH];
} subscription;

typedef struct {
    subscription pool[30000];
    int nextFree;
} subscriptionList;

struct messageBuffer {
    long mtype;
    char mtext[3*MAX_STRING_LENGTH];
};

OperationResult subscribe(subscriptionList *subscriptionList, int subscriberPID, char key[MAX_STRING_LENGTH]);
int notifySubscribers(int messageQueueID, subscriptionList *subscriptionList, char key[MAX_STRING_LENGTH], char message[MAX_STRING_LENGTH]);
int unsubscribeFromAllKeys(subscriptionList *subscriptionList, int oldSubscriberPID);

#endif //BETRIEBSSYSTEME_SUBSCRIPTION_H
