#include <iostream>

extern "C" void doSomeCppThing(void) {
	std::cout << "Hello\n";
}
