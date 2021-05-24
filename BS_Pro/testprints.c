#include <stdio.h>
#include "testPrints.h"
#include "keyValStore.h"

int someTestPrintsForPutGetDel() {
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
    char testRes3[200];
    char testRes4[200];

    char* validKey = "ThisShouldBeAV4l1dK3y";
    char* invalidKey = "Hello World amk /*]";


    printf("put() of new value.\n");
    printf("result of put(%s,%s): %i\n", testKey0, testValue0, put(testKey0, testValue0));
    printf("\n");

    printf("get() of existing key-value.\n");
    printf("result of get(%s, testRes0): %i\n", testKey0, get(testKey0, testRes0));
    printf("testRes0 = %s\n", testRes0);
    printf("\n");

    printf("get() of non-existing key-value.\n");
    printf("result of get(%s, testRes1): %i\n", testKey1, get(testKey1, testRes1));
    printf("testRes1 = %s\n", testRes1);
    printf("\n");

    printf("Updates existing key-value.\n");
    printf("result of get(%s, testRes1): %i\n", testKey0, get(testKey0, testRes1));
    printf("testRes1 = %s\n", testRes1);
    printf("result of put(%s,%s): %i\n", testKey0, testValue1, put(testKey0, testValue1));
    printf("result of get(%s, testRes1): %i\n", testKey0, get(testKey0, testRes1));
    printf("testRes1 = %s\n", testRes1);
    printf("\n");

    printf("Put of new key-value. Should not overwrite the existing one.\n");
    printf("result of put(%s,%s): %i\n", testKey2, testValue2, put(testKey2, testValue2));
    printf("result of get(%s, testRes1): %i\n", testKey2, get(testKey2, testRes2));
    printf("testRes2 = %s\n", testRes2);
    printf("\n");

    printf("Del of existing key.\n");
    printf("result of get(%s, testRes3): %i\n", testKey0, get(testKey0, testRes3));
    printf("testRes3 = %s\n", testRes3);
    printf("result of del(%s) = %i\n", testKey0, del(testKey0));
    printf("result of get(%s, testRes4): %i\n", testKey0, get(testKey0, testRes4));
    printf("testRes4 = %s\n", testRes4);
    printf("\n");

    printf("Del of non-existing key.\n");
    printf("result of del(%s) = %i\n", "brathahn123", del("brathahn123"));
    printf("\n");

    printf("isValidKeyOrValue-test\n");
    printf("%s is valid: %i\n", validKey, isValidKeyOrValue(validKey));
    printf("%s is valid: %i\n", invalidKey, isValidKeyOrValue(invalidKey));

    return 0;
}

