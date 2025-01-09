# TinySMTP

A tiny library to send email using SMTP. \
It only has the basic feature to send a email to a single recipient. \
It doesn't have: CC, or CCO, or any other advanced email feature. \
It uses TLS to connect to SMTP server. \
It was tested in linux and zephyr RTOS. \
For new platforms, you need to implement the `socket` interface, defined at `transport.h` file.
