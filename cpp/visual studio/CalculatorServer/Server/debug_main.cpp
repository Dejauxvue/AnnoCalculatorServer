#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment(lib, "WS2_32.lib")


#include "image_recognition.hpp"

int main() {
	const auto result = image_recognition::get_anno_population_tesserarct_ocr(
		image_recognition::load_image("image_recon/test_screenshots/pop_island_sa_dark_1680.png"));
	std::cout << "result: " << std::endl;
	for (const auto& l : result) {
		std::cout << l.first << ": " << l.second << std::endl;
	}
	std::cout << std::endl;
	return 0;
}