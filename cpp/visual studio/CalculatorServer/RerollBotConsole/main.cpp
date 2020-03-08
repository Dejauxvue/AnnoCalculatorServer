#pragma once


#include "reader_trading.hpp"
#include "mouse.hpp"

using namespace reader;

std::map<unsigned int, std::set<unsigned int>> relevant_offerings({
	{ (unsigned int) phrase::ARCHIBALD_HARBOUR, { // Archibald Harbour
		240000, // propeller
		4 * 65000, // millicent's manifesto
		//4 * 45575, // Bente Vacation Act
		//4 * 52750, // O'Mara's Regulations
		4 * 21750, // Vindication Women's Rights
	}},
	{(unsigned int) phrase::ELI_HARBOUR, { // Eli Harbour
		4 * 7300, // Magnetist
		//4 * 6250, // Dredger
		//4 * 18750, // Elictrical Engineer
		4 * 68200, // Pyrphorian Whizz
		4 * 83250, // Dario
		4 * 72350, // Feras
		//4 * 47500, // Malching's Back-to-Back
		4 * 56000, // Prenatal Perservation
		//4 * 69575, // Mr. Garrick
		4 * 73750 // Lisowski
		//4 * 75350, // Kadijah
		//4 * 62500, // Salima
		//4 * 50000, // Smokestack Act
		//4 * 22175 // First-Rate Sapper
	}},
	{ (unsigned int) phrase::KAHINA_HARBOUR, { // Kahina Harbour
		4 * 62500, // Salima
		4 * 75350, // Kadijah
		4 * 24750, // Purple Pitcher Plant
		4 * 5600, // Marshland - Water Lily
		4 * 24750, // Marshland - Marsh-Mallow
		4 * 1075 // Marshland - Common Reed
	}},
		{ (unsigned int)phrase::ANNE_HARBOUR, { // Kahina Harbour
		4 * 62500, // Salima
		4 * 75350, // Kadijah
	//	4 * 18750, // Elictrical Engineer
		4 * 6250 // Dredger
	}},
	{ (unsigned int)phrase::INUIT_HARBOUR, { // Kahina Harbour
//		4 * 16225, // Arctic Fox
		4 * 15625, // Musc Ox
		4 * 17175 // Arctic Caribou
	}}
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
		unsigned int trader = reader.get_open_trader();

		if (trader && reader.has_reroll() && reader.can_buy() &&
			relevant_offerings.find(trader) != relevant_offerings.end())
		{
     			const auto offerings = reader.get_offerings();

			if (offerings != prev_offerings)
			{
				prev_offerings = offerings;
				std::list<unsigned int> indices;
				const auto& prices = relevant_offerings.at(reader.get_open_trader());

				for (const offering& off : offerings)
					if (prices.find(off.price) != prices.end() &&
						(trader != (unsigned int) phrase::ELI_HARBOUR || !reader.is_book(off.index)) &&
						(trader != (unsigned int) phrase::ARCHIBALD_HARBOUR || off.price != 87000 || reader.is_book(off.index)))
					{
						indices.push_front(off.index);
						std::cout << "Buying item " << off.index << " with price " << off.price << std::endl;
					}
				

				if (indices.empty())
				{
					mous.click(image_recognition::get_center(trading_params::pane_prev_reroll));
					std::this_thread::sleep_for(std::chrono::milliseconds(250));
					continue;
				}
				else {
					mous.click(image_recognition::get_center(trading_params::pane_prev_exchange));
					int tries = 0;
					while (!reader.is_trading_menu_open())
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(250));
						cv::Mat screenshot = recog.take_screenshot(window);
						reader.update(language, screenshot);
						if (tries++ >= 5)
						{
							std::cout << "Did not detect open trading menu." << std::endl;
							break;
						}
					}

					
					bool paused = false;
					for (unsigned int index : indices)
					{
						reader.update(language, recog.take_screenshot(window));
						if (!reader.can_buy(index) && !paused)
						{
							std::cout << "Cannot add item " << index << " to cart. Please check ship and hit enter to continue." << std::endl;
							system("PAUSE");
							paused = true;
							break;
						}

						mous.click(image_recognition::get_center(reader.get_rel_location(index)));
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(400));
					cv::Mat screenshot = recog.take_screenshot(window);
					reader.update(language, screenshot);

					if (!reader.is_trading_menu_open() || !reader.can_buy())
					{
						if (!paused)
						{
							std::cout << "Cannot execute trade. Please check ship and hit enter to continue." << std::endl;
							system("PAUSE");
						}
						continue;
					}

					mous.click(image_recognition::get_center(trading_params::pane_menu_execute));
					std::this_thread::sleep_for(std::chrono::milliseconds(800));
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