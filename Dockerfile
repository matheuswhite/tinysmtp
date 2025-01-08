FROM zephyrprojectrtos/zephyr-build:v0.26.12

WORKDIR "/workdir/tinysmtp"

COPY west.yml .
RUN west init -l . && cd .. && west update && west zephyr-export

WORKDIR "/workdir/"
RUN git clone https://github.com/zephyrproject-rtos/net-tools && sudo apt-get update -y && sudo apt-get install -y iputils-ping
RUN sudo apt install libssl-dev
