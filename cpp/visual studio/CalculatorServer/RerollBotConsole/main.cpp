#pragma once


#include "reader_trading.hpp"
#include "clicker.hpp"

using namespace reader;

std::set<unsigned int> prices({
		240000, // propeller
		4 * 65000, // millicent's manifesto
		4 * 45575, // Bente Vacation Act
		4 * 52750, //O'Mara's Regulations
		4 * 21750, // Vindication Women's Rights

		4 * 7300, // Magnetist
		4 * 18750, // Elictrical Engineer
		4 * 68200, // Pyrphorian Whizz
		4 * 83250, // Dario
		4 * 72350, // Feras
		4 * 47500, //Malching's Back-to-Back
		4 * 56000, // Prenatal Perservation
		4 * 69575, // Mr. Garrick
		4 * 73750, // Lisowski
		4 * 62500, // Salima
		4 * 75350 // Kadijah
	});

int main() {
	image_recognition recog;
	trading_menu reader(recog);
	cv::Rect2i window = recog.find_anno();
	if (!window.area())
		window = recog.get_desktop();
	mouse mous(recog.get_desktop(), window);

	std::vector<offering> prev_offerings;
	std::string language("english");

	while (true)
	{
		reader.update(language, recog.take_screenshot(window));
		if (reader.get_open_trader() && reader.has_reroll() && reader.can_buy())
		{
			const auto offerings = reader.get_offerings();

			if (offerings != prev_offerings)
			{
				prev_offerings = offerings;
				std::list<unsigned int> indices;

				for (const offering& off : offerings)
					if (prices.find(off.price) != prices.end())
						indices.push_front(off.index);

				if (indices.empty())
				{
					mous.click(image_recognition::get_center(trading_params::pane_prev_reroll));
					std::this_thread::sleep_for(std::chrono::milliseconds(200));
					continue;
				}
				else {
					mous.click(image_recognition::get_center(trading_params::pane_prev_exchange));
					int tries = 0;
					while (!reader.is_trading_menu_open())
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(500));
						cv::Mat screenshot = recog.take_screenshot(window);
						reader.update(language, screenshot);
						if (tries++ >= 5)
						{
							std::cout << "Did not detect open trading menu." << std::endl;
							break;
						}
					}

					

					for (unsigned int index : indices)
					{
						reader.update(language, recog.take_screenshot(window));
						if (!reader.can_buy(index))
						{
							std::cout << "Cannot add item " << index << " to cart. Please check ship and hit enter to continue." << std::endl;
							char tmp;
							std::cin >> tmp;
							break;
						}

						mous.click(image_recognition::get_center(reader.get_rel_location(index)));
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(200));
					cv::Mat screenshot = recog.take_screenshot(window);
					reader.update(language, screenshot);

					if (!reader.is_trading_menu_open() || !reader.can_buy())
					{
						std::cout << "Cannot execute trade. Please check ship and hit enter to continue." << std::endl;
						char tmp;
						std::cin >> tmp;
						break;
					}

					mous.click(image_recognition::get_center(trading_params::pane_menu_execute));
					std::this_thread::sleep_for(std::chrono::milliseconds(500));
					continue;
				}
			}
			else
			{
				std::cout << "Offered items do not seem to have changed. Retry in 1s." << std::endl;
			}
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}


	return 0;
}