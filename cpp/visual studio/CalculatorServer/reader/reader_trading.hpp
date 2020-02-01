#pragma once

#include "reader_util.hpp"

namespace reader
{

class trading_params
{
public:

};

class trading_menu
{
public:
	trading_menu(image_recognition& recog);

	void update(const std::string& language, const cv::Mat& img);

private:
	image_recognition& recog;

	unsigned int open_trader;
};

}