cmake_minimum_required(VERSION 3.19)
project(Betriebssysteme C)

set(CMAKE_C_STANDARD 11)

add_library(lib BS_Pro/server.c
        )

TARGET_LINK_LIBRARIES(lib -lpthread -lrt)
TARGET_LINK_LIBRARIES(lib pthread)

include_directories(BS_Pro)

add_executable(Betriebssysteme
        BS_Pro/keyValStore.c
        BS_Pro/keyValStore.h
        BS_Pro/server.c
        BS_Pro/server.h
        BS_Pro/testprints.c
        BS_Pro/testPrints.h
        BS_Pro/Subscription.c
        BS_Pro/Subscription.h)
