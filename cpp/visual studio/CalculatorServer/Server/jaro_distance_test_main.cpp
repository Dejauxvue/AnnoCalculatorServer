
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment(lib, "WS2_32.lib")

#include "image_recognition.hpp"

void jaro_test(const std::string& a, const std::string& b) {
	std::cout << "jaro_similarity(" << a << ", " << b << "): " << image_recognition::jaro_similarity(a, b) << std::endl;
}

int main() {
	jaro_test("Bauem", "Bauern");
}