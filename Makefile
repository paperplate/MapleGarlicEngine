.PHONY: clean build rebuild all

export VK_LAYER_PATH=~/../linuxbrew/.linuxbrew/share/vulkan/explicit_layer.d

setVars:
	echo $$VK_LAYER_PATH

compileShaders:
	glslangValidator -V shaders/shader.vert.glsl -o shaders/shader.vert.spv
	glslangValidator -V shaders/shader.frag.glsl -o shaders/shader.frag.spv

clean:
	rm -rf build

build:
	CC="zig cc" CXX="zig c++" cmake -B./build -S. -G Ninja ;\
		cd build; \
		ninja ; \
		mv compile_commands.json ..
	

run: build
	cd build; \
	./VulkanTest

rebuild: | clean build

all: | clean run
