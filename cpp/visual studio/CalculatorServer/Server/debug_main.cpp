#pragma once

#include <boost/assert.hpp>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment(lib, "WS2_32.lib")


#include "image_recognition.hpp"

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

void unit_tests(image_recognition& image_recog)
{
	{
		image_recog.update("german", image_recognition::load_image("image_recon/test_screenshots/Anno 1800 Res 2560x1080.png"));
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
		image_recog.update("english", image_recognition::load_image("image_recon/test_screenshots/pop_global_bright_1920.png"));
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
		image_recog.update("english", image_recognition::load_image("image_recon/test_screenshots/pop_global_dark_1680.png"));
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
		image_recog.update("english", image_recognition::load_image("image_recon/test_screenshots/pop_global_dark_1920.png"));
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
		image_recog.update("german", image_recognition::load_image("image_recon/test_screenshots/stat_pop_global_1.png"));
		auto result = image_recog.get_population_amount();
		BOOST_ASSERT(result.at(15000002) == 3360);
		BOOST_ASSERT(result.at(15000003) == 6108);
		BOOST_ASSERT(result.at(15000004) == 6800);
		BOOST_ASSERT(result.at(15000005) == 3752);
		BOOST_ASSERT(result.at(15000006) == 5008);
		BOOST_ASSERT(result.at(112642) == 48);

		result = image_recog.get_assets_existing_buildings();
		BOOST_ASSERT(result.at(15000002) == 112);
		BOOST_ASSERT(result.at(15000003) == 156);
		BOOST_ASSERT(result.at(15000004) == 136);
		BOOST_ASSERT(result.at(15000005) == 376);
		BOOST_ASSERT(result.at(15000006) == 272);
		BOOST_ASSERT(result.at(112642) == 12);
	}

	{
		image_recog.update("german", image_recognition::load_image("image_recon/test_screenshots/stat_pop_global_2.png"));
		auto result = image_recog.get_population_amount();
		BOOST_ASSERT(result.at(15000000) == 5372);
		BOOST_ASSERT(result.at(15000001) == 7664);
		BOOST_ASSERT(result.at(15000002) == 3356);
		BOOST_ASSERT(result.at(15000003) == 6108);
		BOOST_ASSERT(result.at(15000004) == 6800);
		BOOST_ASSERT(result.at(15000005) == 3752);

		result = image_recog.get_assets_existing_buildings();
		BOOST_ASSERT(result.at(15000000) == 541);
		BOOST_ASSERT(result.at(15000001) == 392);
		BOOST_ASSERT(result.at(15000002) == 112);
		BOOST_ASSERT(result.at(15000003) == 156);
		BOOST_ASSERT(result.at(15000004) == 136);
		BOOST_ASSERT(result.at(15000005) == 376);
	}

	std::cout << "all tests passed!" << std::endl;
}

int main() {
	image_recognition image_recog;
//	unit_tests(image_recog);

	cv::Mat src = image_recognition::load_image("C:/Users/Nico/Documents/Dokumente/Computer/Softwareentwicklung/AnnoCalculatorServer/calculator-recognition-issues/widescreen/Anno_18002019-12-27-17-27-14.png");

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

	std::cout << "Optimal Productivity" << std::endl;
	print(image_recog.get_optimal_productivities(), image_recog.get_dictionary().factories);
	std::cout << std::endl;

	std::cout << "Existing factories" << std::endl;
	print(image_recog.get_assets_existing_buildings(), image_recog.get_dictionary().factories);
	std::cout << std::endl;



	return 0;
}