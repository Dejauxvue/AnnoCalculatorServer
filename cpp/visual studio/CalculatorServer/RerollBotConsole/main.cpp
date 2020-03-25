#pragma once

#include <set>

#include "reader_util.hpp"
#include "reader_trading.hpp"
#include "mouse.hpp"

using namespace reader;

std::set<unsigned int> relevant_items({
	191831, // propeller
		192484, // millicent's manifesto
		//191802, // Bente Vacation Act
		//191799, // O'Mara's Regulations
		192483, // Vindication Women's Rights

		//190682, // Travel agent

		191375, // Magnetist
		//191587, // Dredger
		//191376, // Elictrical Engineer
		191377, // Pyrphorian Whizz
		191424, // Dario
		192450, // Feras
		//191805, // Malching's Back-to-Back
		191817, // Prenatal Perservation
		//111167, // Mr. Garrick
		192440, // Lisowski
		//191450, // Kadijah
		//191622, // Salima
		//191686, // Smokestack Act
		//190759 // First-Rate Sapper
	
	
		111111, // Marshland - Water Lily
		112733, // Marshland - Marsh-Mallow
		111112 // Marshland - Common Reed

	});


void test_screenshot(image_recognition& recog, trading_menu& reader)
{
	reader.update("german", recog.load_image("test_screenshots/trading_eli_6.png"));
	auto offerings = reader.get_offerings();

	std::cout << std::endl;

	for (const offering& off : offerings)
	{
		std::cout << off.index << ": " << off.price;
		for(const auto& item : off.item_candidates)
			try {
			std::cout << " " << recog.get_dictionary().items.at(item->guid) << " (" << item->guid << ")";
		}
		catch (const std::exception & e)
		{
		}
		std::cout << std::endl;
	}

	std::cout << std::endl << "Capped items:" << std::endl;

	offerings = reader.get_capped_items();

	for (const offering& off : offerings)
	{
		std::cout << off.index << ": " << off.price;
		for (const auto& item : off.item_candidates)
			try {
			std::cout << " " << recog.get_dictionary().items.at(item->guid) << " (" << item->guid << ")";
		}
		catch (const std::exception & e)
		{
		}
		std::cout << std::endl;
	}
}

template <class _Rep, class _Period>
bool wait_and_try(std::chrono::duration<_Rep,_Period> duration,
	std::function<bool()> condition)
{
	std::this_thread::sleep_for(duration);

	int counter = 0;
	while (!condition() && counter++ < 3)
	{
		duration *= 2;
		std::this_thread::sleep_for(duration);
	}

	return counter <= 3;
};

int main() {
	image_recognition recog;
	trading_menu reader(recog);

//	test_screenshot(recog, reader);

	cv::Rect2i window = recog.find_anno();
	if (!window.area())
		window = recog.get_desktop();
	mouse mous(recog.get_desktop(), window);

	std::vector<offering> prev_offerings;
	std::set<unsigned int> relevant_traders;
	std::string language("english");

	for (unsigned int guid : relevant_items)
	{
		for(unsigned int trader_guid : recog.items.at(guid)->traders)
			relevant_traders.emplace(trader_guid);
	}

	int counter = 0;

	while (true)
	{
		reader.update(language, recog.take_screenshot(window));
		unsigned int trader = reader.get_open_trader();

 		if (trader && reader.has_reroll() &&
			relevant_traders.find(trader) != relevant_traders.end() &&
			!reader.is_ship_full())
		{
        	const auto offerings = reader.get_offerings();

			if (offerings != prev_offerings)
			{
				prev_offerings = offerings;
				std::list<const offering*> purchase_candidates;

				for (const offering& off : offerings)
				{
					//std::cout << off.index << ": " << recog.get_dictionary().items.at(off.item_candidates.front()->guid) << std::endl;
					for (const auto& item : off.item_candidates)
					{
						if (relevant_items.find(item->guid) != relevant_items.end())
						{
							purchase_candidates.push_front(&off);
//							std::cout << "Buying item " << off.index << " with price " << off.price << std::endl;
							break;
						}
					}
				}
				
				//std::cout << std::endl;

				if (purchase_candidates.empty())
				{
					mous.click(image_recognition::get_center(trading_params::pane_menu_reroll));
					std::this_thread::sleep_for(std::chrono::milliseconds(300));
					continue;
				}
				else {
					
					for (const offering* off : purchase_candidates)
					{
						reader.update(language, recog.take_screenshot(window));
						if (reader.is_ship_full())
						{
							break;
						}

						
						mous.click(image_recognition::get_center(off->box));
					}

					std::this_thread::sleep_for(std::chrono::milliseconds(300));
					cv::Mat screenshot = recog.take_screenshot(window);
					reader.update(language, screenshot);

					if (!reader.is_trading_menu_open() || !reader.can_buy())
					{
						std::cout << "Cannot execute trade. Please check ship and hit enter to continue." << std::endl;
					}

					mous.click(image_recognition::get_center(trading_params::pane_menu_execute));
					std::this_thread::sleep_for(std::chrono::milliseconds(200));

					continue;
				}
			}
			else
			{
//				std::cout << "Offered items do not seem to have changed. Retry in 1s." << std::endl;
				if (++counter > 3)
				{
					counter = 0;
					mous.click(image_recognition::get_center(trading_params::pane_menu_reroll));
				}
			}
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}


	return 0;
}