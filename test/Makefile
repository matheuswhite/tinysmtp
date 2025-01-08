.PHONY: linux zephyr

all: linux zephyr

linux:
	mkdir -p build/linux && \
		cd build/linux && \
		cmake ../../linux -DCMAKE_BUILD_TYPE=Debug && \
		make
	build/linux/test_tinysmtp
	@echo "[ OK] Linux"

zephyr:
	mkdir -p build/zephyr
	west build -p -b native_sim/native/64 -d build/zephyr zephyr
	west build -d build/zephyr -t run
	@echo "[ OK] Zephyr"

clean: clean_linux clean_zephyr

clean_linux:
	rm -rf build/linux

clean_zephyr:
	rm -rf build/zephyr