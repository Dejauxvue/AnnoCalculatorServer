#pragma once

#include <boost/assert.hpp>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment(lib, "WS2_32.lib")


#include <chrono>
#include <iostream>

#include "reader_statistics.hpp"

using namespace reader;

template<typename T>
void print(const std::map<unsigned int, T>& map,
	const std::map<unsigned int, std::string>& dictionary = std::map<unsigned int, std::string>())
{
	for (const auto& entry : map)
	{
		if (dictionary.find(entry.first) != dictionary.cend())
			std::cout << dictionary.find(entry.first)->second << "\t=\t" << entry.second << std::endl;
		else
			std::cout << entry.first << "\t=\t" << entry.second << std::endl;
	}
}

void unit_tests(class statistics& image_recog)
{
	auto start = std::chrono::high_resolution_clock::now();
	{
		image_recog.update("german", image_recognition::load_image("test_screenshots/Anno 1800 Res 2560x1080.png"));
		const auto result = image_recog.get_population_amount();
		BOOST_ASSERT(result.at(15000000) == 4510);
		BOOST_ASSERT(result.at(15000001) == 6140);
		BOOST_ASSERT(result.at(15000002) == 960);
		BOOST_ASSERT(result.at(15000003) == 0);
		BOOST_ASSERT(result.at(15000004) == 0);
		BOOST_ASSERT(result.at(15000005) == 0);
		BOOST_ASSERT(result.at(15000006) == 0);
	}
	{
		image_recog.update("english", image_recognition::load_image("test_screenshots/pop_global_bright_1920.png"));
		const auto result = image_recog.get_population_amount();
		BOOST_ASSERT(result.at(15000000) == 1345);
		BOOST_ASSERT(result.at(15000001) == 4236);
		BOOST_ASSERT(result.at(15000002) == 4073);
		BOOST_ASSERT(result.at(15000003) == 11214);
		BOOST_ASSERT(result.at(15000004) == 174699);
		BOOST_ASSERT(result.at(15000005) == 2922);
		BOOST_ASSERT(result.at(15000006) == 8615);
	}
	{
		image_recog.update("english", image_recognition::load_image("test_screenshots/pop_global_dark_1680.png"));
		const auto result = image_recog.get_population_amount();
		BOOST_ASSERT(result.at(15000000) == 1345);
		BOOST_ASSERT(result.at(15000001) == 4236);
		BOOST_ASSERT(result.at(15000002) == 4073);
		BOOST_ASSERT(result.at(15000003) == 11275);
		BOOST_ASSERT(result.at(15000004) == 174815);
		BOOST_ASSERT(result.at(15000005) == 2922);
		BOOST_ASSERT(result.at(15000006) == 8615);
	}
	{
		image_recog.update("english", image_recognition::load_image("test_screenshots/pop_global_dark_1920.png"));
		const auto result = image_recog.get_population_amount();
		BOOST_ASSERT(result.at(15000000) == 1307);
		BOOST_ASSERT(result.at(15000001) == 4166);
		BOOST_ASSERT(result.at(15000002) == 4040);
		BOOST_ASSERT(result.at(15000003) == 10775);
		BOOST_ASSERT(result.at(15000004) == 167805);
		BOOST_ASSERT(result.at(15000005) == 2856);
		BOOST_ASSERT(result.at(15000006) == 8477);
	}

	{
		image_recog.update("english", image_recognition::load_image("test_screenshots/pop_island_artisans_1920.png"));
		const auto result = image_recog.get_population_amount();
		BOOST_ASSERT(result.at(15000000) == 1460);
		BOOST_ASSERT(result.at(15000001) == 2476);
		BOOST_ASSERT(result.at(15000002) == 24);
		BOOST_ASSERT(result.at(15000003) == 0);
		BOOST_ASSERT(result.at(15000004) == 0);
		BOOST_ASSERT(result.at(15000005) == 0);
		BOOST_ASSERT(result.at(15000006) == 0);
	}

	//{
	//	image_recog.update("german", image_recognition::load_image("test_screenshots/stat_pop_global_1.png"));
	//	auto result = image_recog.get_population_amount();
	//	BOOST_ASSERT(result.at(15000002) == 3360);
	//	BOOST_ASSERT(result.at(15000003) == 6108);
	//	BOOST_ASSERT(result.at(15000004) == 6800);
	//	BOOST_ASSERT(result.at(15000005) == 3752);
	//	BOOST_ASSERT(result.at(15000006) == 5008);
	//	BOOST_ASSERT(result.at(112642) == 48);

	//	result = image_recog.get_assets_existing_buildings();
	//	BOOST_ASSERT(result.at(15000002) == 112);
	//	BOOST_ASSERT(result.at(15000003) == 156);
	//	BOOST_ASSERT(result.at(15000004) == 136);
	//	BOOST_ASSERT(result.at(15000005) == 376);
	//	BOOST_ASSERT(result.at(15000006) == 272);
	//	BOOST_ASSERT(result.at(112642) == 12);
	//}

	//{
	//	image_recog.update("german", image_recognition::load_image("test_screenshots/stat_pop_global_2.png"));
	//	auto result = image_recog.get_population_amount();
	//	BOOST_ASSERT(result.at(15000000) == 5372);
	//	BOOST_ASSERT(result.at(15000001) == 7664);
	//	BOOST_ASSERT(result.at(15000002) == 3356);
	//	BOOST_ASSERT(result.at(15000003) == 6108);
	//	BOOST_ASSERT(result.at(15000004) == 6800);
	//	BOOST_ASSERT(result.at(15000005) == 3752);

	//	result = image_recog.get_assets_existing_buildings();
	//	BOOST_ASSERT(result.at(15000000) == 541);
	//	BOOST_ASSERT(result.at(15000001) == 392);
	//	BOOST_ASSERT(result.at(15000002) == 112);
	//	BOOST_ASSERT(result.at(15000003) == 156);
	//	BOOST_ASSERT(result.at(15000004) == 136);
	//	BOOST_ASSERT(result.at(15000005) == 376);
	//}

	{
		image_recog.update("english", image_recognition::load_image("test_screenshots/stat_pop_island_1.png"));
		auto result = image_recog.get_population_amount();
		BOOST_ASSERT(result.at(15000005) == 790);
		BOOST_ASSERT(result.at(15000006) == 518);

		result = image_recog.get_assets_existing_buildings();
		BOOST_ASSERT(result.at(15000005) == 79);
		BOOST_ASSERT(result.at(15000006) == 37);
	}

	{
		image_recog.update("english", image_recognition::load_image("test_screenshots/stat_pop_island_2.png"));
		auto result = image_recog.get_population_amount();
		BOOST_ASSERT(result.at(15000000) == 2097);
		BOOST_ASSERT(result.at(15000001) == 2480);
		BOOST_ASSERT(result.at(15000002) == 2100);
		BOOST_ASSERT(result.at(15000003) == 3040);
		BOOST_ASSERT(result.at(15000004) == 42);

		result = image_recog.get_assets_existing_buildings();
		BOOST_ASSERT(result.at(15000000) == 210);
		BOOST_ASSERT(result.at(15000001) == 124);
		BOOST_ASSERT(result.at(15000002) == 70);
		BOOST_ASSERT(result.at(15000003) == 76);
		BOOST_ASSERT(result.at(15000004) == 1);
	}

	{
		image_recog.update("english", image_recognition::load_image("test_screenshots/stat_pop_island_3.png"));
		auto result = image_recog.get_population_amount();
		BOOST_ASSERT(result.at(15000000) == 1460);
		BOOST_ASSERT(result.at(15000001) == 2480);
		BOOST_ASSERT(result.at(15000002) == 24);
		BOOST_ASSERT(result.at(15000003) == 3040);
		BOOST_ASSERT(result.at(15000004) == 42);

		result = image_recog.get_assets_existing_buildings();
		BOOST_ASSERT(result.at(15000000) == 146);
		BOOST_ASSERT(result.at(15000001) == 124);
		BOOST_ASSERT(result.at(15000002) == 1);
		BOOST_ASSERT(result.at(15000003) == 76);
		BOOST_ASSERT(result.at(15000004) == 1);
	}

	{
		image_recog.update("english", image_recognition::load_image("test_screenshots/stat_pop_island_4.png"));
		auto result = image_recog.get_population_amount();
		BOOST_ASSERT(result.at(15000000) == 1450);
		BOOST_ASSERT(result.at(15000001) == 2474);
		BOOST_ASSERT(result.at(15000002) == 47);


		result = image_recog.get_assets_existing_buildings();
		BOOST_ASSERT(result.at(15000000) == 145);
		BOOST_ASSERT(result.at(15000001) == 124);
		BOOST_ASSERT(result.at(15000002) == 2);
	}

	{
		image_recog.update("english", image_recognition::load_image("test_screenshots/stat_pop_island_5.png"));
		auto result = image_recog.get_population_amount();
		BOOST_ASSERT(result.at(15000000) == 1430);
		BOOST_ASSERT(result.at(15000001) == 2443);
		BOOST_ASSERT(result.at(15000002) == 114);
		BOOST_ASSERT(result.at(15000003) == 3040);
		BOOST_ASSERT(result.at(15000004) == 42);

		result = image_recog.get_assets_existing_buildings();
		BOOST_ASSERT(result.at(15000000) == 143);
		BOOST_ASSERT(result.at(15000001) == 123);
		BOOST_ASSERT(result.at(15000002) == 5);
		BOOST_ASSERT(result.at(15000003) == 76);
		BOOST_ASSERT(result.at(15000004) == 1);
	}


	{
		image_recog.update("english", image_recognition::load_image("test_screenshots/stat_pop_global_widescreen.png"));
		auto result = image_recog.get_assets_existing_buildings();
		BOOST_ASSERT(result.at(15000001) == 335);
		BOOST_ASSERT(result.at(15000002) == 200);
		BOOST_ASSERT(result.at(15000003) == 32);
		BOOST_ASSERT(result.at(15000004) == 0);
		BOOST_ASSERT(result.at(15000005) == 86);
		BOOST_ASSERT(result.at(15000006) == 59);
		BOOST_ASSERT(result.at(112642) == 0);
		BOOST_ASSERT(result.at(112643) == 0);

		result = image_recog.get_population_amount();
		BOOST_ASSERT(result.at(15000001) == 6514);
		BOOST_ASSERT(result.at(15000002) == 5337);
		BOOST_ASSERT(result.at(15000003) == 1069);
		BOOST_ASSERT(result.at(15000004) == 0);
		BOOST_ASSERT(result.at(15000005) == 568);
		BOOST_ASSERT(result.at(15000006) == 1003);
		BOOST_ASSERT(result.at(112642) == 0);
		BOOST_ASSERT(result.at(112643) == 0);
	}

	{
		image_recog.update("german", image_recognition::load_image("test_screenshots/stat_pop_global_3_16_10.jpg"));
		auto result = image_recog.get_assets_existing_buildings();
		BOOST_ASSERT(result.at(15000000) == 145);
		BOOST_ASSERT(result.at(15000001) == 89);
		BOOST_ASSERT(result.at(15000002) == 0);
		BOOST_ASSERT(result.at(15000003) == 0);
		BOOST_ASSERT(result.at(15000004) == 0);
		BOOST_ASSERT(result.at(15000005) == 0);
		BOOST_ASSERT(result.at(112642) == 0);
		BOOST_ASSERT(result.at(112643) == 0);

		result = image_recog.get_population_amount();
		BOOST_ASSERT(result.at(15000000) == 1440);
		BOOST_ASSERT(result.at(15000001) == 1754);
		BOOST_ASSERT(result.at(15000002) == 0);
		BOOST_ASSERT(result.at(15000003) == 0);
		BOOST_ASSERT(result.at(15000004) == 0);
		BOOST_ASSERT(result.at(15000005) == 0);
		BOOST_ASSERT(result.at(112642) == 0);
		BOOST_ASSERT(result.at(112643) == 0);
	}


	/*
	{
		image_recog.update("english", image_recognition::load_image("test_screenshots/stat_pop_island_3.png"));
		auto result = image_recog.get_population_amount();
		BOOST_ASSERT(result.at(15000000) == 1460);
		BOOST_ASSERT(result.at(15000001) == 2480);
		BOOST_ASSERT(result.at(15000002) == 24);
		BOOST_ASSERT(result.at(15000003) == 3040);
		BOOST_ASSERT(result.at(15000004) == 42);
		BOOST_ASSERT(result.at(15000005) == 3752);
		BOOST_ASSERT(result.at(15000006) == 3752);
		BOOST_ASSERT(result.at(112642) == 48);
		BOOST_ASSERT(result.at(112643) == 48);

		result = image_recog.get_assets_existing_buildings();
		BOOST_ASSERT(result.at(15000000) == 146);
		BOOST_ASSERT(result.at(15000001) == 124);
		BOOST_ASSERT(result.at(15000002) == 1);
		BOOST_ASSERT(result.at(15000003) == 76);
		BOOST_ASSERT(result.at(15000004) == 1);
		BOOST_ASSERT(result.at(15000005) == 376);
		BOOST_ASSERT(result.at(15000006) == 3752);
		BOOST_ASSERT(result.at(112642) == 48);
		BOOST_ASSERT(result.at(112643) == 48);
	}
	*/

	std::cout << "all tests passed!" << std::endl;

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration = end - start;
	std::cout << duration.count() << " ms" << std::endl;
}

int main(int argc, char** argv) {
	image_recognition recog(true);
	statistics image_recog(recog);
	//unit_tests(image_recog);

//	cv::Mat src = image_recognition::load_image("C:/Users/Nico/Documents/Anno 1800/screenshot/screenshot_2019-12-31-13-03-20.jpg");
//	cv::Mat src = image_recognition::load_image("C:/Users/Nico/Pictures/Uplay/Anno 1800/Anno 18002020-1-6-0-32-3.png");
//	cv::Mat src = image_recognition::load_image("C:/Users/Nico/Documents/Dokumente/Computer/Softwareentwicklung/AnnoCalculatorServer/calculator-recognition-issues/population_number_slash_issue/screenshot6.png");
//	 	cv::Mat src = image_recognition::load_image("J:/Pictures/Uplay/Anno 1800/Anno 18002020-1-6-0-32-3.png");

	cv::Mat src = argc >= 2 ? image_recognition::load_image(argv[1]) : image_recognition::load_image("test_screenshots/stat_prod_global_3_16_10.jpg");

	image_recog.update("english", src);

	//image_recog.update("german", image_recognition::load_image("C:/Users/Nico/Documents/Dokumente/Computer/Softwareentwicklung/AnnoCalculatorServer/calculator-recognition-issues/island_name_mua/screenshot.png"));

	std::string island = image_recog.get_selected_island();
	std::cout << "Island: " << island << std::endl;
	std::cout << std::endl;

	const auto& islands = image_recog.get_islands();
	std::cout << "Island list: " << std::endl;
	for (const auto& entry : islands)
		try {
		std::cout << entry.first << "\t" << image_recog.get_dictionary().ui_texts.at(entry.second) << std::endl;
	}
	catch (const std::exception& e) {}
	std::cout << std::endl;

	std::cout << "Population amount" << std::endl;
	print(image_recog.get_population_amount(), image_recog.get_dictionary().population_levels);
	std::cout << std::endl;


	std::cout << "Average Productivity" << std::endl;
	print(image_recog.get_average_productivities(), image_recog.get_dictionary().factories);
	std::cout << std::endl;

	//std::cout << "Optimal Productivity" << std::endl;
	//print(image_recog.get_optimal_productivities(), image_recog.get_dictionary().factories);
	//std::cout << std::endl;

	std::cout << "Existing factories" << std::endl;
	print(image_recog.get_assets_existing_buildings(), image_recog.get_dictionary().factories);
	std::cout << std::endl;



	return 0;
}