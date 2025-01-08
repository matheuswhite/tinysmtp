cmake_minimum_required(VERSION 3.22)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(OPENSSL_LIB /usr/lib/aarch64-linux-gnu/libssl.a /usr/lib/aarch64-linux-gnu/libcrypto.a)

project(test_tinysmtp)

include_directories(../../include)
file(GLOB SOURCES
    ../../src/linux/sockets.c
    ../../src/transport.c
    main.c
)

add_executable(test_tinysmtp ${SOURCES})
target_link_libraries(test_tinysmtp ${OPENSSL_LIB})
