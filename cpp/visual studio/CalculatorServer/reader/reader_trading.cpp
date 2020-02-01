#include "reader_trading.hpp"

reader::trading_menu::trading_menu(image_recognition& recog)
	:
	recog(recog),
	open_trader(0)
{
}

void reader::trading_menu::update(const std::string& language, const cv::Mat& img)
{
	open_trader = 0;

	recog.update(language, img);
}
