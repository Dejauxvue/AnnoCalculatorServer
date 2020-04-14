#include <chrono>

#include "../CalculatorServer/RerollBotConsole/mouse.hpp"
#include "configuration.hpp"
#include "../CalculatorServer/reader/reader_trading.hpp" 
#include "../CalculatorServer/reader/reader_trading.hpp"

class execution_result
{
public:
	std::chrono::duration<long long, std::milli> wait_time;
	std::chrono::time_point<std::chrono::steady_clock> finished;
	bool updated_recognition;

	execution_result(std::chrono::duration<long long, std::milli> wait_time = std::chrono::milliseconds(0),
		bool updated_recognition = true);
};

class bot
{
public:
	bot(configuration& config, reader::image_recognition& recog, bool verbose = false);
	bot() = delete;

	virtual ~bot();

	virtual execution_result execute_step(bool update_required = true);

protected:
	configuration& config;
	reader::image_recognition& recog;
	cv::Rect2i window;
	mouse mous;
	bool verbose;

	int screenshot_counter = 1;

	cv::Mat take_screenshot();
	void log_offerings(const std::vector<reader::offering>& offerings);
};

class reroll_bot : public bot
{
public:
	reroll_bot(configuration& config, reader::image_recognition& recog, bool verbose = false);

	virtual execution_result execute_step(bool update_required = true) override;

private:
	reader::trading_menu reader;
	std::map<unsigned int, unsigned int> reroll_costs;
	cv::Rect2i window;
	
	std::vector<reader::offering> prev_offerings;
	int counter = 0;

	std::string get_time_str() const;
};