#include "subscription.h"

int getSubscriptionIndex(subscriptionList *subscriptionList, int subscriberPID, char *key) {
    int currentPosition = 0;
    while (currentPosition < subscriptionList->nextFree) {
        subscription currentSubscription = subscriptionList->pool[currentPosition];
        if (currentSubscription.subscriberPID == subscriberPID && strcmp(currentSubscription.key, key) == 0) {
            return currentPosition;
        }
        currentPosition++;
    }
    return -1;
}

int getFirstSubscriptionIndexOfSubscriber(subscriptionList *subscriptionList, int subscriberPID) {
    int currentPosition = 0;
    while (currentPosition < subscriptionList->nextFree) {
        subscription currentSubscription = subscriptionList->pool[currentPosition];
        if (currentSubscription.subscriberPID == subscriberPID) {
            return currentPosition;
        }
        currentPosition++;
    }
    return -1;
}

// critical operation on subscriptionList
OperationResult subscribe(subscriptionList *subscriptionList, int newSubscriberPID, char key[MAX_STRING_LENGTH]) {
    OperationResult result;
    memset(result.message, '\0', sizeof(result.message));

    char dummy[MAX_STRING_LENGTH];
    if (get(key, dummy).code == -1) {
        result.code = -1;
        strcpy(result.message, "key_nonexistent");
    } else if (getSubscriptionIndex(subscriptionList, newSubscriberPID, key) != -1) {
        result.code = -2;
        strcpy(result.message, "already_subbed");
    } else if (subscriptionList->nextFree >= ( sizeof(subscriptionList->pool) / sizeof(subscriptionList->pool[0]) )) {
        result.code = -3;
        strcpy(result.message, "reached_global_subscription_limit");
    } else {
        memset(subscriptionList->pool[subscriptionList->nextFree].key, '\0', sizeof(subscriptionList->pool[subscriptionList->nextFree].key));
        strcpy(subscriptionList->pool[subscriptionList->nextFree].key, key);
        subscriptionList->pool[subscriptionList->nextFree].subscriberPID = newSubscriberPID;
        subscriptionList->nextFree = subscriptionList->nextFree + 1;

        result.code = 0;
        strcpy(result.message, "subbed_to_key");
    }
    return result;
}

int removeSubscriptionByIndex(subscriptionList *subscriptionList, int index) {
    if (index >= subscriptionList->nextFree) {
        return 0;
    }
    while (index < subscriptionList->nextFree-1) {
        subscriptionList->pool[index].subscriberPID = subscriptionList->pool[index + 1].subscriberPID;
        memset(subscriptionList->pool[index].key, '\0', sizeof(subscriptionList->pool[index].key));
        strcpy(subscriptionList->pool[index].key, subscriptionList->pool[index+1].key);
        index++;
    }
    subscriptionList->nextFree = subscriptionList->nextFree -1;
    return 1;
}

// critical operation on subscriptionList
int unsubscribeFromAllKeys(subscriptionList *subscriptionList, int oldSubscriberPID) {
    OperationResult result;
    memset(result.message, '\0', sizeof(result.message));

    int subscriptionIndex, numOfRemovedSubscriptions = 0;
    while ((subscriptionIndex = getFirstSubscriptionIndexOfSubscriber(subscriptionList, oldSubscriberPID)) != -1) {
        numOfRemovedSubscriptions += removeSubscriptionByIndex(subscriptionList, subscriptionIndex);
    }
    return numOfRemovedSubscriptions;
}

int notifySubscribers(int messageQueueID, subscriptionList *subscriptionList, char key[MAX_STRING_LENGTH], char message[3*MAX_STRING_LENGTH]) {
    int numOfSentMessages = 0, currentPosition = 0;

    struct messageBuffer newMessage;
    memset(newMessage.mtext, '\0', sizeof(newMessage.mtext));
    strcpy(newMessage.mtext, message);

    while (currentPosition < subscriptionList->nextFree) {
        subscription currentSubscription = subscriptionList->pool[currentPosition];
        if (strcmp(currentSubscription.key, key) == 0) {
            newMessage.mtype = currentSubscription.subscriberPID;
            msgsnd(messageQueueID, &newMessage, strlen(newMessage.mtext)+1, 0);
            numOfSentMessages++;
        }
        currentPosition++;
    }
    return numOfSentMessages;
}
