#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment(lib, "WS2_32.lib")


#include "image_recognition.hpp"

int main() {
	const auto result = image_recognition::get_anno_population_tesserarct_ocr(
		image_recognition::load_image("image_recon/test_screenshots/Anno 1800 Res 2560x1080.png"));
	std::cout << std::endl;
	return 0;
}