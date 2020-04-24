#include <iostream>
#include <thread>

#include <vcclr.h>
#include <msclr/marshal_cppstd.h>

#include "../CalculatorServer/reader/reader_util.hpp"
#include "../CalculatorServer/reader/reader_trading.hpp"
#include "../CalculatorServer/reader/version.hpp"

#include "configuration.hpp"
#include "bots.hpp"
#include "gui.hpp"


using namespace System;
using namespace System::Threading;

using namespace AssetViewer;





void run_bots(bool* termination_flag, std::vector<std::shared_ptr<bot>>& bots)
{
	std::chrono::time_point<std::chrono::steady_clock> next_invokation = std::chrono::high_resolution_clock::now();
	while (!(*termination_flag))
	{
		if (next_invokation > std::chrono::high_resolution_clock::now())
			std::this_thread::sleep_until(next_invokation);

		next_invokation = std::chrono::high_resolution_clock::now() + std::chrono::minutes(1);
		bool update_required = true;
		for (auto& bot : bots)
		{
			try {
				auto result = bot->execute_step(update_required);
				next_invokation = std::min(next_invokation, result.finished + result.wait_time);
				update_required = !result.updated_recognition;
			}
			catch (std::exception& e)
			{
				std::cout << e.what() << std::endl;
			}
		}
	}
}


int main(int argc, char* argv[])
{
	bool verbose = false;
	if (argc > 1 && std::strcmp(argv[1], "-v") == 0)
		verbose = true;

	std::cout << "Initializing ... ";

	reader::version::check_and_log();

	reader::image_recognition recog(verbose);
	reader::trading_menu trade(recog);

	configuration config(recog, "UXEnhancerConfig.json");

	std::vector<std::shared_ptr<bot>> bots({
		std::shared_ptr<bot>(new reroll_bot(config, recog, verbose))
		});

	bool termination_flag = false;
	std::thread bot_thread(&run_bots, &termination_flag, bots);

	std::cout << "Done" << std::endl;

	Gui^ gui_thread = gcnew Gui(config);
	
	gui_thread->join();

	termination_flag = true;
	bot_thread.join();
}

