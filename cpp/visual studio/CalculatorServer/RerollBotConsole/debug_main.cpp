#pragma once


#include "reader_trading.hpp"

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


int main() {
	image_recognition recog;
	trading_menu reader(recog);

	//	cv::Mat src = image_recognition::load_image("C:/Users/Nico/Documents/Anno 1800/screenshot/screenshot_2019-12-31-13-03-20.jpg");
	//	cv::Mat src = image_recognition::load_image("C:/Users/Nico/Pictures/Uplay/Anno 1800/Anno 18002020-1-6-0-32-3.png");
	// cv::Mat src = image_recognition::load_image("C:/Users/Nico/Documents/Dokumente/Computer/Softwareentwicklung/AnnoCalculatorServer/calculator-recognition-issues/population_number_slash_issue/screenshot6.png");
	//	 	cv::Mat src = image_recognition::load_image("J:/Pictures/Uplay/Anno 1800/Anno 18002020-1-6-0-32-3.png");
	cv::Mat src = image_recognition::load_image("test_screenshots/trade_eli_1.png");

	reader.update("english", src);

	//image_recog.update("german", image_recognition::load_image("C:/Users/Nico/Documents/Dokumente/Computer/Softwareentwicklung/AnnoCalculatorServer/calculator-recognition-issues/island_name_mua/screenshot.png"));

	try {
		std::cout << "Trader: " << recog.get_dictionary().ui_texts.at(reader.get_open_trader()) << std::endl;
		std::cout << "Rerollable: " << reader.has_reroll() << std::endl;
		std::cout << "Buyable: " << reader.can_buy() << std::endl;

		const auto offerings = reader.get_offerings();
		std::cout << "Offerings:" << std::endl;
		for (const auto& offering : offerings)
			std::cout << offering.index << ": " << offering.price << std::endl;
	}
	catch (const std::exception & e)
	{	}

	return 0;
}