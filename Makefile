.PHONY: clean build all

clean:
	rm -rf build

build:
	cmake -B./build -S. ;\
		cd build; \
		ninja ;

run: build
	cd build; \
	./VulkanTest

all: | clean run
