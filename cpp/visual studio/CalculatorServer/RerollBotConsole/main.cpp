#pragma once

#include <boost/filesystem.hpp>
#include <set>

#include <boost/date_time.hpp>


#include "reader_util.hpp"
#include "reader_trading.hpp"
#include "mouse.hpp"
#include "wishlist.hpp"

using namespace reader;

void log_offerings(const image_recognition& recog, const std::vector<offering>& offerings)
{
	for (const offering& off : offerings)
	{
		std::cout << off.index << ": " << off.price;
		for (const auto& item : off.item_candidates)
			try {
			std::cout << " " << recog.get_dictionary().items.at(item->guid) << " (" << item->guid << ")";
		}
		catch (const std::exception& e)
		{
		}
		std::cout << std::endl;
	}
}

std::string get_time_str()
{
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();

	std::stringstream stream;

	stream << "[" 
		<< time.time_of_day().hours()
		<< ":" << time.time_of_day().minutes()
		<< ":" << time.time_of_day().seconds()
		<< "." << time.time_of_day().fractional_seconds()
		<< "] ";

	return stream.str();
}

void test_screenshot(image_recognition& recog, trading_menu& reader)
{
	reader.update("english", recog.load_image("test_screenshots/trading_kahina_2.png"));
	auto offerings = reader.get_offerings(true);

	std::cout << std::endl;

	log_offerings(recog, offerings);
	

	std::cout << std::endl << "Capped items:" << std::endl;

	offerings = reader.get_capped_items();

	log_offerings(recog, offerings);
}

template <class _Rep, class _Period>
bool wait_and_try(std::chrono::duration<_Rep, _Period> duration,
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

int main(int argc, char** argv) {
	try {
		bool verbose = false;
		if (argc > 1 && std::strcmp(argv[1], "-v") == 0)
			verbose = true;



		std::cout << "Initializing ...";
		image_recognition recog(verbose);
		trading_menu reader(recog);
		item_wishlist wishlist(recog, "RerollbotConfig.json");
		std::cout << "Done" << std::endl;



		std::map<unsigned int, unsigned int> reroll_costs;
		for (const auto& entry : recog.trader_to_offerings)
			reroll_costs.emplace(entry.first, 0);

		// test_screenshot(recog, reader);

		cv::Rect2i window = recog.find_anno();
		if (!window.area())
			window = recog.get_desktop();
		mouse mous(recog.get_desktop(), window);

		std::vector<offering> prev_offerings;

		int screenshot_counter = 1;
		auto take_screenshot = [&]()
		{
			cv::Mat screenshot(recog.take_screenshot(window));
			if (verbose)
			{
				boost::filesystem::create_directory("debug_images");
				cv::imwrite("debug_images/screenshot-" + std::to_string(screenshot_counter) + ".png", screenshot);
				if (screenshot_counter++ > 20)
					screenshot_counter = 1;
			}
			return screenshot;

		};

		int counter = 0;

		while (true)
		{
			reader.update(wishlist.get_language(), take_screenshot());
			unsigned int trader = reader.get_open_trader();

			if (trader && reader.has_reroll() &&
				wishlist.buy_from(trader) &&
				!reader.is_ship_full())
			{
				unsigned int reroll_cost = reader.get_reroll_cost();
				if (reroll_cost)
					reroll_costs[trader] = reroll_cost;

				std::vector<offering> offerings = reader.get_offerings(counter <= 3);

				if (offerings != prev_offerings && offerings.size())
				{
					
					if (verbose)
						try {
						std::cout << "Trader: " << recog.get_dictionary().traders.at(trader) << std::endl;
						log_offerings(recog, offerings);
					}
					catch (const std::exception& e) {}

					counter = 0;

					if (!reroll_cost && wishlist.get_max_reroll_costs() &&
						(!reroll_costs[trader] || reroll_costs[trader] + 5000 > wishlist.get_max_reroll_costs()))
					{
						if (verbose)
							std::cout << get_time_str() << "Check reroll costs (internal: " << reroll_costs[trader] << ")" << std::endl;
							
						mous.move(image_recognition::get_center(trading_params::pane_menu_reroll));
						std::this_thread::sleep_for(std::chrono::milliseconds(250));
						continue;
					}

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

					if (verbose)
					{
						std::cout << "purchase: ";
						for (const auto& entry : purchase_candidates)
							std::cout << entry->item_candidates.front()->guid << " ";
						std::cout << reroll_costs[trader] << ")" << std::endl;
					}

					//std::cout << std::endl;

					if (purchase_candidates.empty())
					{
						if (verbose)
							std::cout << get_time_str() << "No desired item found";

						if (!wishlist.get_max_reroll_costs() ||
							reroll_costs[trader] < wishlist.get_max_reroll_costs())
						{
							if (verbose)
								std::cout << " -> Reroll" << std::endl;

							mous.click(image_recognition::get_center(trading_params::pane_menu_reroll));
							std::this_thread::sleep_for(std::chrono::milliseconds(250));
						}
						else if (verbose)
						{
							std::cout << " but reroll costs exceeded: " << reroll_costs[trader] << std::endl;
						}
						continue;
					}
					else {

						auto purchase_iter = purchase_candidates.begin();
						for (; purchase_iter != purchase_candidates.end(); ++purchase_iter)
						{
							reader.update(wishlist.get_language(), take_screenshot());
							if (reader.is_ship_full())
							{
								if (verbose)
									std::cout << get_time_str() << "Ship full -> abort" << std::endl;
								break;
							}


							mous.click(image_recognition::get_center((*purchase_iter)->box));
						}

						int exceute_check_count = 0;
						do
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(300));
							cv::Mat screenshot = take_screenshot();
							reader.update(wishlist.get_language(), screenshot);

							if (verbose)
								std::cout << get_time_str() << "try executing trade (" << exceute_check_count << ")" << std::endl;

						} while ((!reader.is_trading_menu_open() || !reader.can_buy()) && exceute_check_count++ < 4);

						if (exceute_check_count == 5)
						{
							std::cout << "Cannot execute trade. Proceed with rerolling." << std::endl;
							continue;
						}

						mous.click(image_recognition::get_center(trading_params::pane_menu_execute));
						for (auto iter = purchase_candidates.begin(); iter != purchase_iter; ++iter)
						{
							for (const auto& candidate : (*iter)->item_candidates)
								wishlist.bought(candidate->guid);
						}

						if (verbose)
							std::cout << get_time_str() << "Wait for trade executed" << std::endl;
						std::this_thread::sleep_for(std::chrono::milliseconds(400));

						continue;
					}
				}
				else
				{
					if (verbose)
						std::cout << get_time_str() << "Identical Offerings at: " << trader << ", counter: " << counter << std::endl;

					//				std::cout << "Offered items do not seem to have changed. Retry in 1s." << std::endl;
					if (++counter > 3)
					{
						counter = 0;
						if (!wishlist.get_max_reroll_costs() ||
							reroll_costs[trader] < wishlist.get_max_reroll_costs())
						{
							if (verbose)
								std::cout << "Tries exceeded -> Reroll" << std::endl;
							mous.click(image_recognition::get_center(trading_params::pane_menu_reroll));
						}
						else if (verbose)
						{
							std::cout << get_time_str() << "Reroll costs exceeded: " << reroll_costs[trader] << std::endl;
						}
					}

					if (!offerings.size())
					{
						if (verbose)
							std::cout << "Wait for item fully loaded" << std::endl;

						// wait for item being fully loaded
						std::this_thread::sleep_for(std::chrono::milliseconds(100));
						continue;
					}
				}

			} else if (verbose)
				std::cout << get_time_str() << "Trader: " << trader << ", ship full: " << reader.is_ship_full() << ", has reroll button: " << reader.has_reroll() << std::endl;
			
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