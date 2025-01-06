cmake_minimum_required(VERSION 3.22)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(test_tinysmtp)

set(OPENSSL_LIB /opt/homebrew/lib/libssl.a /opt/homebrew/lib/libcrypto.a)
set(OPENSSL_INCLUDE /opt/homebrew/include/)

include_directories(../.. ${OPENSSL_INCLUDE})

file(GLOB SOURCES ../../linux/sockets.c main.c ../../transport.c)

add_executable(test_tinysmtp ${SOURCES})

target_link_libraries(test_tinysmtp ${OPENSSL_LIB})
