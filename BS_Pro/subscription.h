
#ifndef UNTITLED_SUBSCRIPTION_H
#define UNTITLED_SUBSCRIPTION_H

#include <sys/types.h>
#include "keyValStore.h"

struct subscription {
    pid_t subscriberId;
    char subscribedKey;
    struct subscription *next;
    struct subscription *previous;
};

OperationResult subscribe(pid_t *, char *);

void deleteClientSubscription(pid_t *);

void deleteKeySubscription(char *);

void publishChanges(char *, const char *);

#endif //UNTITLED_SUBSCRIPTION_H
