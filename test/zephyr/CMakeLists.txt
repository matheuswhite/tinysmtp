# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
set(BOARD native_sim/native/64)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(tinysmtp)

include_directories(../..)

target_sources(app PRIVATE main.c ../../zephyr/sockets.c ../../transport.c)

set(gen_dir ${ZEPHYR_BINARY_DIR}/include/generated/)
generate_inc_file_for_target(
    app
    ../../zephyr/globalsign_r1.der
    ${gen_dir}/globalsign_r1.der.inc
    )
