#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment(lib, "WS2_32.lib")


#include "image_recognition.hpp"

int main() {
	image_recognition::get_anno_population_tesserarct_ocr(image_recognition::load_image("image_recon/test_screenshots/screenshot_2019-05-19-12-32-45.jpg"));
	return 0;
}