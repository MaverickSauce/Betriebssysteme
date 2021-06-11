#define MAX_MESSAGE_LENGTH 256

#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include "Subscription.h"

struct subscription *subscription_head = NULL;

// Returns a pointer to an already existing subscription
struct subscription *get_subscription(pid_t *subscriberId, char *subscribedKey) {
    struct subscription *subscription_i = subscription_head;
    while (subscription_i != NULL) {
        if (subscription_i->subscribedKey == subscribedKey && subscription_i->subscriberId == subscriberId)
            return subscription_i;
        subscription_i = subscription_i->next;
    }
    return NULL;
}

// add subscription to the list
void subscribe(pid_t *subscriberId, char *subscribedKey) {
    if (get_subscription(subscriberId, subscribedKey) != NULL) { //prevents the same user from subscribing to the same key twice
        return;
    }
    struct subscription *subscription_i = NULL;
    if ((subscription_i = calloc(1, sizeof(struct subscription))) == NULL) { //
        return;
    }

    subscription_i->subscriberId = subscriberId;
    subscription_i->subscribedKey = subscribedKey;

    subscription_i->next = subscription_head;
    if (subscription_head != NULL)
        subscription_head->previous = subscription_i;
    subscription_head = subscription_i;
}

//remove client subscriptions after quitting
void deleteClientSubscription(pid_t *subscriberId) {
    struct subscription *subscription_i = subscription_head;
    while (subscription_i != NULL) {
        if (subscriberId == subscription_i->subscriberId) {
            if (subscription_i->next != NULL) {
                subscription_i->next->previous = subscription_i->previous;
            }
            if (subscription_i->previous != NULL) {
                subscription_i->previous->next = subscription_i->next;
            }
            if (subscription_i == subscription_head) {
                subscription_head = subscription_i->next;
            }
        }
        subscription_i = subscription_i->next;
    }
}

//remove client subscriptions after deleting a key
void deleteKeySubscription(char *subscribedKey) {
    struct subscription *subscription_i = subscription_head;
    while (subscription_i != NULL) {
        if (subscribedKey == subscription_i->subscribedKey) {
            if (subscription_i->next != NULL) {
                subscription_i->next->previous = subscription_i->previous;
            }
            if (subscription_i->previous != NULL) {
                subscription_i->previous->next = subscription_i->next;
            }
            if (subscription_i == subscription_head) {
                subscription_head = subscription_i->next;
            }
        }
        subscription_i = subscription_i->next;
    }
}

//publish changes to subscribers of a specific key
void publishChanges(char *subscribedKey, const char *message) {
    struct subscription *subscription_i = subscription_head;
    while (subscription_i != NULL) {
        if (subscribedKey == subscription_i->subscribedKey) {
            msgsnd(subscription_i->subscriberId, message, MAX_MESSAGE_LENGTH, 1);
            subscription_i = subscription_i->next;
        }
        subscription_i = subscription_i->next;
    }
}
