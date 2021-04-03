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

void unit_tests(image_recognition& recog, statistics& image_recog)
{
	auto start = std::chrono::high_resolution_clock::now();


	auto test_image = [&](const std::string& language, const std::string& path, std::map<unsigned int, statistics_screen::properties> expected)
	{
		auto get_name = [&](unsigned int guid)
		{
			const auto& dict = recog.dictionaries.at(language);
			auto iter = dict.population_levels.find(guid);

			if (iter != dict.population_levels.end())
			{
				return iter->second;
			}

			iter = dict.factories.find(guid);

			if (iter != dict.factories.end())
			{
				return iter->second;
			}

			iter = dict.products.find(guid);

			if (iter != dict.products.end())
			{
				return iter->second;

			}

			return std::to_string(guid);
		};

		image_recog.update(language, image_recognition::load_image(path));
		const auto result = image_recog.get_all();

		for (const auto& asset : result)
		{
			auto expected_iter = expected.find(asset.first);
			if (expected_iter == expected.end())
				if (asset.second.find(statistics_screen::KEY_AMOUNT) != asset.second.end() && asset.second.at(statistics_screen::KEY_AMOUNT) == 0)
					continue;
				else {
					std::cout << path << " [FP] " << get_name(asset.first);

					for (const auto& entry : asset.second)
						std::cout << "\t" << entry.first << ": " << entry.second;

					std::cout << std::endl;
				}
			else
			{
				for (const auto& entry_expected : expected_iter->second) {
					const auto& iter_actual = asset.second.find(entry_expected.first);

					if (iter_actual == asset.second.end())
						std::cout << path << " [MISS] " << get_name(asset.first) << "." << entry_expected.first << " expected " << entry_expected.second << std::endl;
					else if (iter_actual->second != entry_expected.second)
						std::cout << path << " [DIFF] " << get_name(asset.first) << "." << entry_expected.first << " expected " << entry_expected.second << " got " << iter_actual->second << std::endl;
				}
				expected.erase(expected_iter);
			}
		}

		for (const auto& asset : expected)
		{
			std::cout << path << " [FN] " << get_name(asset.first);

			for (const auto& entry : asset.second)
				std::cout << "\t" << entry.first << ": " << entry.second;

			std::cout << std::endl;
		}
	};
	using properties = statistics_screen::properties;
	{
		test_image("german", "test_screenshots/Anno 1800 Res 2560x1080.png",
			std::map<unsigned int, properties>({
			{15000000, properties({{statistics_screen::KEY_AMOUNT, 4510}}) },
				{ 15000001, properties({{statistics_screen::KEY_AMOUNT, 6140}}) },
				{ 15000002, properties({{statistics_screen::KEY_AMOUNT, 960}}) },
				{ 15000003, properties({{statistics_screen::KEY_AMOUNT, 0}}) },
				{ 15000004, properties({{statistics_screen::KEY_AMOUNT, 0}}) },
				{ 15000005, properties({{statistics_screen::KEY_AMOUNT, 0}}) },
				{ 15000006, properties({{statistics_screen::KEY_AMOUNT, 0}}) }
				}));
	}
	{
		test_image("english", "test_screenshots/pop_global_bright_1920.png",
			std::map<unsigned int, properties>({
			{15000000, properties({{statistics_screen::KEY_AMOUNT, 1345}})},
			{15000001, properties({{statistics_screen::KEY_AMOUNT, 4236}})},
			{15000002, properties({{statistics_screen::KEY_AMOUNT, 4073}})},
			{15000003, properties({{statistics_screen::KEY_AMOUNT, 11214}})},
			{15000004, properties({{statistics_screen::KEY_AMOUNT, 174699}})},
			{15000005, properties({{statistics_screen::KEY_AMOUNT, 2922}})},
			{15000006, properties({{statistics_screen::KEY_AMOUNT, 8615}})}
				}));
	}
	{
		test_image("english", "test_screenshots/pop_global_dark_1680.png",
			std::map<unsigned int, properties>({
			{15000000, properties({{statistics_screen::KEY_AMOUNT, 1345}})},
			{15000001, properties({{statistics_screen::KEY_AMOUNT, 4236}})},
			{15000002, properties({{statistics_screen::KEY_AMOUNT, 4073}})},
			{15000003, properties({{statistics_screen::KEY_AMOUNT, 11275}})},
			{15000004, properties({{statistics_screen::KEY_AMOUNT, 174815}})},
			{15000005, properties({{statistics_screen::KEY_AMOUNT, 2922}})},
			{15000006, properties({{statistics_screen::KEY_AMOUNT, 8615}})}
				}));
	}
	{
		test_image("english", "test_screenshots/pop_global_dark_1920.png",
			std::map<unsigned int, properties>({
			{15000000, properties({{statistics_screen::KEY_AMOUNT, 1307}})},
			{15000001, properties({{statistics_screen::KEY_AMOUNT, 4166}})},
			{15000002, properties({{statistics_screen::KEY_AMOUNT, 4040}})},
			{15000003, properties({{statistics_screen::KEY_AMOUNT, 10775}})},
			{15000004, properties({{statistics_screen::KEY_AMOUNT, 167805}})},
			{15000005, properties({{statistics_screen::KEY_AMOUNT, 2856}})},
			{15000006, properties({{statistics_screen::KEY_AMOUNT, 8477}})} }));
	}

	{
		test_image("english", "test_screenshots/pop_island_artisans_1920.png",
			std::map<unsigned int, properties>({
			{15000000, properties({{statistics_screen::KEY_AMOUNT, 1460}})},
			{15000001, properties({{statistics_screen::KEY_AMOUNT, 2476}})},
			{15000002, properties({{statistics_screen::KEY_AMOUNT, 24}})},
			{15000003, properties({{statistics_screen::KEY_AMOUNT, 0}})},
			{15000004, properties({{statistics_screen::KEY_AMOUNT, 0}})},
			{15000005, properties({{statistics_screen::KEY_AMOUNT, 0}})},
			{15000006, properties({{statistics_screen::KEY_AMOUNT, 0}})} }));
	}

	{
		test_image("english", "test_screenshots/stat_pop_island_1.png",
			std::map<unsigned int, properties>({
			{15000005, properties({{statistics_screen::KEY_AMOUNT, 790},{statistics_screen::KEY_EXISTING_BUILDINGS, 79}})},
			{15000006, properties({{statistics_screen::KEY_AMOUNT, 518},{statistics_screen::KEY_EXISTING_BUILDINGS, 37}})} }));

	}

	{
		test_image("english", "test_screenshots/stat_pop_island_2.png",
			std::map<unsigned int, properties>({
			{15000000, properties({{statistics_screen::KEY_AMOUNT, 2097},{statistics_screen::KEY_EXISTING_BUILDINGS, 210}})},
			{15000001, properties({{statistics_screen::KEY_AMOUNT, 2480},{statistics_screen::KEY_EXISTING_BUILDINGS, 124}})},
			{15000002, properties({{statistics_screen::KEY_AMOUNT, 2100},{statistics_screen::KEY_EXISTING_BUILDINGS, 70}})},
			{15000003, properties({{statistics_screen::KEY_AMOUNT, 3040},{statistics_screen::KEY_EXISTING_BUILDINGS, 76}})},
			{15000004, properties({{statistics_screen::KEY_AMOUNT, 42},{statistics_screen::KEY_EXISTING_BUILDINGS, 1}})} }));

	}

	{
		test_image("english", "test_screenshots/stat_pop_island_3.png",
			std::map<unsigned int, properties>({
			{15000000, properties({{statistics_screen::KEY_AMOUNT, 1460},{statistics_screen::KEY_EXISTING_BUILDINGS, 146}})},
			{15000001, properties({{statistics_screen::KEY_AMOUNT, 2480},{statistics_screen::KEY_EXISTING_BUILDINGS, 124}})},
			{15000002, properties({{statistics_screen::KEY_AMOUNT, 24},{statistics_screen::KEY_EXISTING_BUILDINGS, 1}})},
			{15000003, properties({{statistics_screen::KEY_AMOUNT, 3040},{statistics_screen::KEY_EXISTING_BUILDINGS, 76}})},
			{15000004, properties({{statistics_screen::KEY_AMOUNT, 42},{statistics_screen::KEY_EXISTING_BUILDINGS, 1}})} }));

	}

	{
		test_image("english", "test_screenshots/stat_pop_island_4.png",
			std::map<unsigned int, properties>({
			{15000000, properties({{statistics_screen::KEY_AMOUNT, 1450},{statistics_screen::KEY_EXISTING_BUILDINGS, 145}})},
			{15000001, properties({{statistics_screen::KEY_AMOUNT, 2474},{statistics_screen::KEY_EXISTING_BUILDINGS, 124}})},
			{15000002, properties({{statistics_screen::KEY_AMOUNT, 47},{statistics_screen::KEY_EXISTING_BUILDINGS, 2}})} }));

	}

	{
		test_image("english", "test_screenshots/stat_pop_island_5.png",
			std::map<unsigned int, properties>({
			{15000000, properties({{statistics_screen::KEY_AMOUNT, 1430},{statistics_screen::KEY_EXISTING_BUILDINGS, 143}})},
			{15000001, properties({{statistics_screen::KEY_AMOUNT, 2443},{statistics_screen::KEY_EXISTING_BUILDINGS, 123}})},
			{15000002, properties({{statistics_screen::KEY_AMOUNT, 114},{statistics_screen::KEY_EXISTING_BUILDINGS, 5}})},
			{15000003, properties({{statistics_screen::KEY_AMOUNT, 3040},{statistics_screen::KEY_EXISTING_BUILDINGS, 76}})},
			{15000004, properties({{statistics_screen::KEY_AMOUNT, 42},{statistics_screen::KEY_EXISTING_BUILDINGS, 1}})} }));


	}


	{
		test_image("english", "test_screenshots/stat_pop_global_widescreen.png",
			std::map<unsigned int, properties>({
			{15000001, properties({{statistics_screen::KEY_EXISTING_BUILDINGS, 335},{statistics_screen::KEY_AMOUNT, 6514}})},
			{15000002, properties({{statistics_screen::KEY_EXISTING_BUILDINGS, 200},{statistics_screen::KEY_AMOUNT, 5337}})},
			{15000003, properties({{statistics_screen::KEY_EXISTING_BUILDINGS, 32},{statistics_screen::KEY_AMOUNT, 1069}})},
			{15000004, properties({{statistics_screen::KEY_EXISTING_BUILDINGS, 0},{statistics_screen::KEY_AMOUNT, 0}})},
			{15000005, properties({{statistics_screen::KEY_EXISTING_BUILDINGS, 86},{statistics_screen::KEY_AMOUNT, 568}})},
			{15000006, properties({{statistics_screen::KEY_EXISTING_BUILDINGS, 59},{statistics_screen::KEY_AMOUNT, 1003}})},
			{112642, properties({{statistics_screen::KEY_EXISTING_BUILDINGS, 0},{statistics_screen::KEY_AMOUNT, 0}})},
			{112643, properties({{statistics_screen::KEY_EXISTING_BUILDINGS, 0},{statistics_screen::KEY_AMOUNT, 0}})} }));

	}

	{
		test_image("german", "test_screenshots/stat_pop_global_3_16_10.jpg",
			std::map<unsigned int, properties>({
			{15000000, properties({{statistics_screen::KEY_EXISTING_BUILDINGS, 145},{statistics_screen::KEY_AMOUNT, 1440}})},
			{15000001, properties({{statistics_screen::KEY_EXISTING_BUILDINGS, 89},{statistics_screen::KEY_AMOUNT, 1754}})},
			{15000002, properties({{statistics_screen::KEY_EXISTING_BUILDINGS, 0},{statistics_screen::KEY_AMOUNT, 0}})},
			{15000003, properties({{statistics_screen::KEY_EXISTING_BUILDINGS, 0},{statistics_screen::KEY_AMOUNT, 0}})},
			{15000004, properties({{statistics_screen::KEY_EXISTING_BUILDINGS, 0},{statistics_screen::KEY_AMOUNT, 0}})},
			{15000005, properties({{statistics_screen::KEY_EXISTING_BUILDINGS, 0},{statistics_screen::KEY_AMOUNT, 0}})},
			{112642, properties({{statistics_screen::KEY_EXISTING_BUILDINGS, 0},{statistics_screen::KEY_AMOUNT, 0}})},
			{112643, properties({{statistics_screen::KEY_EXISTING_BUILDINGS, 0},{statistics_screen::KEY_AMOUNT, 0}})} }));


	}


	std::cout << "all tests passed!" << std::endl;

	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> duration = end - start;
	std::cout << duration.count() << " ms" << std::endl;
}

int main(int argc, char** argv) {
	image_recognition recog(true);
	statistics image_recog(recog);
	//unit_tests(recog, image_recog);

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

	const auto& dict = recog.dictionaries.at("english");
	for (const auto& asset : image_recog.get_all())
	{
		try { std::cout << dict.population_levels.at(asset.first); }
		catch (...) {}
		try { std::cout << dict.factories.at(asset.first); }
		catch (...) {}

		std::cout << ": { ";

		for (const auto& entry : asset.second)
			std::cout << entry.first << ": " << entry.second << ", ";

		std::cout << "}" << std::endl;
	}
	
	return 0;
}