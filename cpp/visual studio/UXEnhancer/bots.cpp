#include "bots.hpp"

#include <boost/date_time.hpp>

#include "../CalculatorServer/reader/reader_util.hpp"


execution_result::execution_result(std::chrono::duration<long long, std::milli> wait_time, bool updated_recognition)
	:
	wait_time(wait_time),
	finished(std::chrono::high_resolution_clock::now()),
	updated_recognition(updated_recognition)
{
}


bot::bot(configuration& config, reader::image_recognition& recog, bool verbose)
	:
	config(config),
	recog(recog),
	window(recog.find_anno().area() ? recog.find_anno() : recog.get_desktop()),
	mous(recog.get_desktop(), window),
	verbose(verbose)
{
}

bot::~bot()
{
}

execution_result bot::execute_step(bool update_required)
{
	throw std::exception("cannot execute step on abstract class bot");
}

cv::Mat bot::take_screenshot()
{
	cv::Mat screenshot(recog.take_screenshot(window));
	if (verbose)
	{
		System::IO::Directory::CreateDirectory("debug_images");
		cv::imwrite("debug_images/screenshot-" + std::to_string(screenshot_counter) + ".png", screenshot);
		if (screenshot_counter++ > 20)
			screenshot_counter = 1;
	}
	return screenshot;
}

void bot::log_offerings(const std::vector<reader::offering>& offerings)
{
	for (const reader::offering& off : offerings)
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


reroll_bot::reroll_bot(configuration& config, reader::image_recognition& recog, bool verbose)
	:
	bot(config, recog, verbose),
	reader(recog)
{
	for (const auto& entry : recog.trader_to_offerings)
		reroll_costs.emplace(entry.first, 0);
}

execution_result reroll_bot::execute_step(bool update_required)
{
	if(update_required)
		reader.update(config.get_language(), take_screenshot());

	unsigned int trader = reader.get_open_trader();

	if (trader && reader.has_reroll() &&
		config.wishlist.buy_from(trader) &&
		!reader.is_ship_full())
	{
		unsigned int reroll_cost = reader.get_reroll_cost();
		if (reroll_cost)
			reroll_costs[trader] = reroll_cost;

		std::vector<reader::offering> offerings = reader.get_offerings(counter <= 3);

		if (offerings != prev_offerings && offerings.size())
		{

			if (verbose)
				try {
				std::cout << "Trader: " << recog.get_dictionary().traders.at(trader) << std::endl;
				log_offerings(offerings);
			}
			catch (const std::exception& e) {}

			counter = 0;

			if (!reroll_cost && config.get_max_reroll_costs() &&
				(!reroll_costs[trader] || reroll_costs[trader] + 5000 > config.get_max_reroll_costs()))
			{
				if (verbose)
					std::cout << get_time_str() << "Check reroll costs (internal: " << reroll_costs[trader] << ")" << std::endl;

				mous.move(reader::image_recognition::get_center(reader::trading_params::pane_menu_reroll));
				
				return execution_result(std::chrono::milliseconds(250), false);
			}

			prev_offerings = offerings;
			std::list<const reader::offering*> purchase_candidates;

			for (const reader::offering& off : offerings)
			{
				//std::cout << off.index << ": " << recog.get_dictionary().items.at(off.item_candidates.front()->guid) << std::endl;
				for (const auto& item : off.item_candidates)
				{
					if (config.wishlist.contains(item->guid))
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

				if (!config.get_max_reroll_costs() ||
					reroll_costs[trader] < config.get_max_reroll_costs())
				{
					if (verbose)
						std::cout << " -> Reroll" << std::endl;

					mous.click(reader::image_recognition::get_center(reader::trading_params::pane_menu_reroll));
					return execution_result(std::chrono::milliseconds(250), false);
				}
				else if (verbose)
				{
					std::cout << " but reroll costs exceeded: " << reroll_costs[trader] << std::endl;
					
				}
				return execution_result();
			}
			else {

				auto purchase_iter = purchase_candidates.begin();
				for (; purchase_iter != purchase_candidates.end(); ++purchase_iter)
				{
					reader.update(config.get_language(), take_screenshot());
					if (reader.is_ship_full())
					{
						if (verbose)
							std::cout << get_time_str() << "Ship full -> abort" << std::endl;
						break;
					}


					mous.click(reader::image_recognition::get_center((*purchase_iter)->box));
				}

				int exceute_check_count = 0;
				do
				{
					System::Threading::Thread::Sleep(TimeSpan::FromMilliseconds(300));
					cv::Mat screenshot = take_screenshot();
					reader.update(config.get_language(), screenshot);

					if (verbose)
						std::cout << get_time_str() << "try executing trade (" << exceute_check_count << ")" << std::endl;

				} while ((!reader.is_trading_menu_open() || !reader.can_buy()) && exceute_check_count++ < 4);

				if (exceute_check_count == 5)
				{
					std::cout << "Cannot execute trade. Proceed with rerolling." << std::endl;
					return execution_result();
				}

				mous.click(reader::image_recognition::get_center(reader::trading_params::pane_menu_execute));
				for (auto iter = purchase_candidates.begin(); iter != purchase_iter; ++iter)
				{
					for (const auto& candidate : (*iter)->item_candidates)
						config.wishlist.bought(candidate->guid);
				}

				if (verbose)
					std::cout << get_time_str() << "Wait for trade executed" << std::endl;
				System::Threading::Thread::Sleep(TimeSpan::FromMilliseconds(400));

				return execute_step();
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
				if (!config.get_max_reroll_costs() ||
					reroll_costs[trader] < config.get_max_reroll_costs())
				{
					if (verbose)
						std::cout << "Tries exceeded -> Reroll" << std::endl;
					mous.click(reader::image_recognition::get_center(reader::trading_params::pane_menu_reroll));
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
				return execution_result(std::chrono::milliseconds(100));
			}
		}

	}
	else if (verbose)
		std::cout << get_time_str() << "Trader: " << trader << ", ship full: " << reader.is_ship_full() << ", has reroll button: " << reader.has_reroll() << std::endl;

	return execution_result(std::chrono::seconds(1));
}

std::string reroll_bot::get_time_str() const
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
