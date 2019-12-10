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

int main() {
	image_recognition image_recog;
	image_recog.update("english", image_recognition::load_image("J:/Pictures/Uplay/Anno 1800 Preview/Anno 1800 Preview2019-9-24-10-53-45.png"));

	std::string island = image_recog.get_selected_island();
	std::cout << "Island: " << island << std::endl;

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
	
	std::cout << "all tests passed!" << std::endl;

	return 0;
}