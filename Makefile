.PHONY: clean build rebuild all

clean:
	rm -rf build

build:
	cmake -B./build -S. ;\
		cd build; \
		ninja ; \
		mv compile_commands.json ..

run: build
	cd build; \
	./VulkanTest

rebuild: | clean build

all: | clean run
