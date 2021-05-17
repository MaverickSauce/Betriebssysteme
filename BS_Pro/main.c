#include <stdio.h>
#include "main.h"

int main() {
    // Random data to check functionality.
    char* testKey0 = "1337abc42";
    char* testValue0 = "HelloWorld";

    char* testKey1 = "4711blub420";
    char* testValue1 = "iLoveBetriebssystemeUndVerteilteSysteme";

    char* testKey2 = "13";
    char* testValue2 = "Nope";

    char testRes0[200];
    char testRes1[200];
    char testRes2[200];


    // Proper unittests would be better. Need get() to actually work.
    // -> Change every put to putAlt to see the difference.

    // put() of new value. get() of existing key-value.
    printf("result of put(%s,%s): %i\n", testKey0, testValue0, put(testKey0, testValue0));
    printf("testRes0 = %s\n", testRes0);
    printf("result of get(%s, testRes0): %i\n", testKey0, get(testKey0, testRes0));
    printf("testRes0 = %s\n", testRes0);
    printf("\n");

    // get() of non-existing key-value.
    printf("testRes0 = %s\n", testRes0);
    printf("result of get(%s, testRes1): %i\n", testKey1, get(testKey1, testRes1));
    printf("testRes0 = %s\n", testRes0);
    printf("\n");

    // Updates existing key-value.
    printf("result of put(%s,%s): %i\n", testKey0, testValue1, put(testKey0, testValue1));
    printf("testRes1 = %s\n", testRes1);
    printf("result of get(%s, testRes1): %i\n", testKey0, get(testKey0, testRes1));
    printf("testRes1 = %s\n", testRes1);
    printf("\n");

    // Put of new key-value. Should not overwrite the existing one.
    printf("result of put(%s,%s): %i\n", testKey2, testValue2, put(testKey2, testValue2));
    printf("testRes2 = %s\n", testRes2);
    printf("result of get(%s, testRes1): %i\n", testKey2, get(testKey2, testRes2));
    printf("testRes2 = %s\n", testRes2);
    printf("\n");

    // del-tests etc.
    printf("result of del(%s) = %i\n", testKey0, del(testKey0));

    return 0;
}
