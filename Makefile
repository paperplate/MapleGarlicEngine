compileShaders:
	slangc shaders/main.frag.slang -target spirv -entry main -stage fragment -o shaders/compiled/main.frag.spv
	slangc shaders/main.vert.slang -target spirv -entry main -stage vertex -o shaders/compiled/main.vert.spv
	mv shaders/compiled/*.spv build/shaders/compiled/
	cd build; ninja
	cd build; ./Tetris
