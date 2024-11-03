#CFLAGS = -std=c++23 -O2
#LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

#VulkanTest: main.cpp
#VulkanTest: helloTriangle.cpp
#VulkanTest: 15_hello_triangle.cpp
	#g++ $(CLFAGS) -o VulkanTest main.cpp $(LDFLAGS)
#	g++ $(CLFAGS) -o VulkanTest helloTriangle.cpp $(LDFLAGS)
	#g++ $(CLFAGS) -o VulkanTest 15_hello_triangle.cpp $(LDFLAGS)

.PHONY: test clean

run:
	cd build; \
		cmake ..; \
		ninja ; \
		./VulkanTest

#test: VulkanTest
#	./VulkanTest

clean:
	rm -f VulkanTest
