#define MAX_MESSAGE_LENGTH 256

#include <stdlib.h>
#include <sys/msg.h>
#include <string.h>
#include "subscription.h"

struct subscription *subscription_head = NULL;

// Returns a pointer to an already existing subscription
struct subscription *get_subscription(const pid_t subscriberId, char *subscribedKey) {
    struct subscription *subscription_i = subscription_head;
    while (subscription_i != NULL) {
        if (strcmp(subscription_i->subscribedKey, subscribedKey) == 0 &&
           subscription_i->subscriberId == subscriberId)
            return subscription_i;
        subscription_i = subscription_i->next;
    }
    return NULL;
}

// add subscription to the list
OperationResult subscribe(pid_t subscriberId, char *subKey) {
    OperationResult result;

    memset(result.message, '\0', sizeof(result.message));

    if (get(subKey, NULL).code == -1) {
        result.code = -2;
        strcpy(result.message, "No such Key");

    } else if (get_subscription(subscriberId, subKey) !=
               NULL) { //prevents the same user from subscribing to the same key twice
        result.code = -1;
        strcpy(result.message, "Already subscribed");

    } else {
        struct subscription *subscription_i = NULL;
        if ((subscription_i = calloc(1, sizeof(struct subscription))) == NULL) { //Subscription memory full
            result.code = -2;
            strcpy(result.message, "subscription unavailable");

        } else {
            memset(subscription_i->subscribedKey, '\0', sizeof(subscription_i->subscribedKey));
            subscription_i->subscriberId = subscriberId;
            strcpy(subscription_i->subscribedKey, subKey);
            subscription_i->next = subscription_head;

            if (subscription_head != NULL)// add subscription if list is not empty
                subscription_head->previous = subscription_i;

            subscription_head = subscription_i; // add subscription if list is empty
            result.code = 0;
            strcpy(result.message, "subscription successful");
        }
    }
    //  0 -> A new key was added.
    // -1 -> Client already subscribed.
    // -2 -> Memory full
    // -3 -> Key doesn't exist
    return result;
}

//remove client subscriptions after quitting
void deleteClientSubscription(pid_t subscriberId) {
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
        if (strcmp(subscribedKey, subscription_i->subscribedKey) == 0) {
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
        if (strcmp(subscribedKey, subscription_i->subscribedKey) == 0) {
            msgsnd(subscription_i->subscriberId, message, MAX_MESSAGE_LENGTH, 1);
            subscription_i = subscription_i->next;
        }
        subscription_i = subscription_i->next;
    }
}
