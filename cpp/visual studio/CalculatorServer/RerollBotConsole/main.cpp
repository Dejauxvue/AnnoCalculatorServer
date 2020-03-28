#pragma once

#include <set>

#include "reader_util.hpp"
#include "reader_trading.hpp"
#include "mouse.hpp"
#include "wishlist.hpp"

using namespace reader;

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
	try {
		image_recognition recog;
		trading_menu reader(recog);
		item_wishlist wishlist(recog, "RerollbotConfig.json");

		//	test_screenshot(recog, reader);

		cv::Rect2i window = recog.find_anno();
		if (!window.area())
			window = recog.get_desktop();
		mouse mous(recog.get_desktop(), window);

		std::vector<offering> prev_offerings;


		int counter = 0;

		while (true)
		{
			reader.update(wishlist.get_language(), recog.take_screenshot(window));
			unsigned int trader = reader.get_open_trader();

			if (trader && reader.has_reroll() &&
				wishlist.buy_from(trader) &&
				!reader.is_ship_full())

				try {
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
							if (wishlist.contains(item->guid))
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
						std::this_thread::sleep_for(std::chrono::milliseconds(250));
						continue;
					}
					else {

						auto purchase_iter = purchase_candidates.begin();
						for (; purchase_iter != purchase_candidates.end(); ++purchase_iter)
						{
							reader.update(wishlist.get_language(), recog.take_screenshot(window));
							if (reader.is_ship_full())
							{
								break;
							}


							mous.click(image_recognition::get_center((*purchase_iter)->box));
						}

						int count = 0;
						do
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(300));
							cv::Mat screenshot = recog.take_screenshot(window);
							reader.update(wishlist.get_language(), screenshot);

						} while ((!reader.is_trading_menu_open() || !reader.can_buy()) && count++ < 2);

						if (count == 3)
						{
							std::cout << "Cannot execute trade. Please check ship and hit enter to continue." << std::endl;
							continue;
						}

						mous.click(image_recognition::get_center(trading_params::pane_menu_execute));
						for (auto iter = purchase_candidates.begin(); iter != purchase_iter; ++iter)
						{
							for (const auto& candidate : (*iter)->item_candidates)
								wishlist.bought(candidate->guid);
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(400));

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
			catch (const std::exception & e)
			{
				// wait for item being fully loaded
				std::this_thread::sleep_for(std::chrono::milliseconds(250));
				continue;
			}

			std::this_thread::sleep_for(std::chrono::seconds(1));
		}

		return 0;
	}
	catch (const std::exception & e)
	{
		std::cout << e.what() << std::endl;
		system("pause");
		return -1;
	}
}