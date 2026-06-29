compileShaders:
	glslangValidator -V shaders/shader.vert.glsl -o shaders/shader.vert.spv
	glslangValidator -V shaders/shader.frag.glsl -o shaders/shader.frag.spv
	mv shaders/*.spv build/shaders/compiled/
