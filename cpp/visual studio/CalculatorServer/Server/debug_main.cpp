#pragma once

#include <boost/assert.hpp>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment(lib, "WS2_32.lib")


#include "image_recognition.hpp"



int main() {
	{
		const auto result = image_recognition::get_anno_population_tesserarct_ocr(
			image_recognition::load_image("image_recon/test_screenshots/Anno 1800 Res 2560x1080.png"));
		BOOST_ASSERT(result.at("farmers") == 4510);
		BOOST_ASSERT(result.at("workers") == 6140);
		BOOST_ASSERT(result.at("artisans") == 960);
		BOOST_ASSERT(result.find("engineers") == result.end());
		BOOST_ASSERT(result.find("investors") == result.end());
		BOOST_ASSERT(result.find("jornaleros") == result.end());
		BOOST_ASSERT(result.find("obreros") == result.end());	
	}
	{
		const auto result = image_recognition::get_anno_population_tesserarct_ocr(
			image_recognition::load_image("image_recon/test_screenshots/pop_global_bright_1920.png"));
		BOOST_ASSERT(result.at("farmers") == 1345);
		BOOST_ASSERT(result.at("workers") == 4236);
		BOOST_ASSERT(result.at("artisans") == 4073);
		BOOST_ASSERT(result.at("engineers") == 11214);
		BOOST_ASSERT(result.at("investors") == 174699);
		BOOST_ASSERT(result.at("jornaleros") == 2922);
		BOOST_ASSERT(result.at("obreros") == 8615);
	}
	{
		const auto result = image_recognition::get_anno_population_tesserarct_ocr(
			image_recognition::load_image("image_recon/test_screenshots/pop_global_dark_1680.png"));
		BOOST_ASSERT(result.at("farmers") == 1345);
		BOOST_ASSERT(result.at("workers") == 4236);
		BOOST_ASSERT(result.at("artisans") == 4073);
		BOOST_ASSERT(result.at("engineers") == 11275);
		BOOST_ASSERT(result.at("investors") == 174815);
		BOOST_ASSERT(result.at("jornaleros") == 2922);
		BOOST_ASSERT(result.at("obreros") == 8615);
	}
	{
		const auto result = image_recognition::get_anno_population_tesserarct_ocr(
			image_recognition::load_image("image_recon/test_screenshots/pop_global_dark_1920.png"));
		BOOST_ASSERT(result.at("farmers") == 1307);
		BOOST_ASSERT(result.at("workers") == 4166);
		BOOST_ASSERT(result.at("artisans") == 4040);
		BOOST_ASSERT(result.at("engineers") == 10775);
		BOOST_ASSERT(result.at("investors") == 167805);
		BOOST_ASSERT(result.at("jornaleros") == 2856);
		BOOST_ASSERT(result.at("obreros") == 8477);
	}
	
	std::cout << "all tests passed!" << std::endl;
	return 0;
}