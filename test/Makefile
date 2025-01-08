.PHONY: linux zephyr

all: linux zephyr

linux:
	@printf "\e[33m***** Building linux variant\e[0m\n"
	mkdir -p build/linux && \
		cd build/linux && \
		cmake ../../linux -DCMAKE_BUILD_TYPE=Debug && \
		make
	@printf "\e[33m***** Running linux variant\e[0m\n"
	build/linux/test_tinysmtp
	@printf "[ \e[1;32mOK\e[0m] Linux\n"

zephyr:
	@printf "\e[33m***** Building zephyr variant\e[0m\n"
	mkdir -p build/zephyr
	west build -p -b native_sim/native/64 -d build/zephyr zephyr
	@printf "\e[33m***** Running zephyr variant\e[0m\n"
	west build -d build/zephyr -t run
	@printf "[ \e[1;32mOK\e[0m] Zephyr\n"

clean: clean_linux clean_zephyr

clean_linux:
	rm -rf build/linux

clean_zephyr:
	rm -rf build/zephyr