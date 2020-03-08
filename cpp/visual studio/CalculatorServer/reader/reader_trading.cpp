#include "reader_trading.hpp"

namespace reader
{

////////////////////////////////////////
//
// Class: trading_params
//
////////////////////////////////////////

const cv::Scalar trading_params::background_marine_blue = cv::Scalar(98, 69, 84, 255);
const cv::Scalar trading_params::background_sand_bright = cv::Scalar(162, 211, 233, 255);
const cv::Scalar trading_params::background_sand_dark = cv::Scalar(135, 183, 211, 255);
const cv::Scalar trading_params::background_grey_bright = cv::Scalar(82, 100, 114, 255);
const cv::Scalar trading_params::background_grey_dark = cv::Scalar(50, 50, 50, 255);
const cv::Scalar trading_params::background_green_bright = cv::Scalar(59, 144, 95, 255);
const cv::Scalar trading_params::background_green_dark = cv::Scalar(66, 92, 62, 255);
const cv::Scalar trading_params::frame_brown = cv::Scalar(89, 152, 195, 255);

const cv::Rect2f trading_params::size_icon = cv::Rect2f(cv::Point2f(0.0185, 0.23), cv::Point2f(0.963,1.));
const cv::Rect2f trading_params::size_price = cv::Rect2f(cv::Point2f(0.218, 0.), cv::Point2f(1., 0.225));
const cv::Rect2f trading_params::size_offering = cv::Rect2f(0,0,0.03545,0.07967);

const unsigned int trading_params::count_cols = 4;
const unsigned int trading_params::count_rows = 2;

const cv::Rect2f trading_params::pane_prev_name = cv::Rect2f(cv::Point2f(0.8386, 0.5676), cv::Point2f(0.974, 0.5942));
const cv::Rect2f trading_params::pane_prev_tab_items = cv::Rect2f(cv::Point2f(0.9148, 0.62), cv::Point2f(0.9827, 0.6462));
const cv::Rect2f trading_params::pane_prev_offering = cv::Rect2f(cv::Point2f(0.8314, 0.661), cv::Point2f(0.9859, 0.8241));
const cv::Rect2f trading_params::pane_prev_reroll = cv::Rect2f(cv::Point2f(0.8954, 0.8354), cv::Point2f(0.924, 0.8864));
const cv::Rect2f trading_params::pane_prev_exchange = cv::Rect2f(cv::Point2f(0.8955, 0.9352), cv::Point2f(0.9241, 0.9862));

const cv::Rect2f trading_params::pane_menu_offering = cv::Rect2f(cv::Point2f(0.6238, 0.4293), cv::Point2f(0.7781, 0.5934));
const cv::Rect2f trading_params::pane_menu_name = cv::Rect2f(cv::Point2f(0.6354, 0.3477), cv::Point2f(0.766, 0.3717));
const cv::Rect2f trading_params::pane_menu_reroll = cv::Rect2f(cv::Point2f(0.6897, 0.7149), cv::Point2f(0.7183, 0.7658));
const cv::Rect2f trading_params::pane_menu_execute = cv::Rect2f(cv::Point2f(0.4397, 0.7793), cv::Point2f(0.5616, 0.8081));
const cv::Rect2f trading_params::pane_menu_title = cv::Rect2f(cv::Point2f(0.4356, 0.1937), cv::Point2f(0.5704, 0.2335));


////////////////////////////////////////
//
// Class: trading_menu
//
////////////////////////////////////////

trading_menu::trading_menu(image_recognition& recog)
	:
	recog(recog),
	open_trader(0),
	menu_open(false)
{
}

void trading_menu::update(const std::string& language, const cv::Mat& img)
{
	open_trader = 0;
	menu_open = false;

	if (!img.cols)
		return;

	recog.update(language, img);

	// test if preview is open
	cv::Mat trader_name = recog.binarize(recog.get_pane(trading_params::pane_prev_name), true);
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/trader_name.png", trader_name);
#endif

	auto trader_candidates = recog.get_guid_from_name(trader_name, recog.make_dictionary({ 
		phrase::ARCHIBALD_HARBOUR, 
		phrase::ANNE_HARBOUR,
		phrase::FORTUNE_HARBOUR,
		phrase::ISABELL_HARBOUR,
		phrase::ELI_HARBOUR,
		phrase::KAHINA_HARBOUR,
		phrase::NATE_HARBOUR,
		phrase::INUIT_HARBOUR
	}));

#ifdef CONSOLE_DEBUG_OUTPUT
		std::cout << std::endl;
#endif

	open_trader = trader_candidates.empty() ? 0 : trader_candidates.front();

	if (!open_trader)
	{
		// test if trading menu is open
		cv::Mat trading_menu_title = recog.binarize(recog.get_pane(trading_params::pane_menu_title), true);

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
		cv::imwrite("debug_images/trading_menu_title.png", trading_menu_title);
#endif

		menu_open = !recog.get_guid_from_name(trading_menu_title, recog.make_dictionary({ phrase::TRADE })).empty();

		if (menu_open)
		{
			trader_name = recog.binarize(recog.get_pane(trading_params::pane_menu_name), true);
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
			cv::imwrite("debug_images/trader_name.png", trader_name);
#endif

			auto trader_candidates = recog.get_guid_from_name(trader_name, recog.make_dictionary({
				phrase::ARCHIBALD,
				phrase::ANNE,
				phrase::FORTUNE,
				phrase::ISABELL,
				phrase::ELI,
				phrase::KAHINA,
				phrase::NATE,
				phrase::INUIT
				}));

			if(!trader_candidates.empty())
				open_trader = trader_candidates.front();
		}
	}

}

bool trading_menu::is_trade_preview_open() const
{
	return !menu_open && open_trader;
}

bool trading_menu::is_trading_menu_open() const
{
	return menu_open;
}

bool trading_menu::has_reroll() const
{
	if (is_trade_preview_open())
		return recog.is_button(recog.get_pane(trading_params::pane_prev_reroll), 
			trading_params::background_marine_blue, 
			trading_params::background_sand_bright);
	
	else if(is_trading_menu_open())
		return recog.is_button(recog.get_pane(trading_params::pane_menu_reroll),
			trading_params::background_marine_blue,
			trading_params::background_sand_bright);

	return false;
}

bool trading_menu::can_buy() const
{
	if (is_trade_preview_open())
		return recog.is_button(recog.get_pane(trading_params::pane_prev_exchange),
			trading_params::background_marine_blue,
			trading_params::background_sand_dark);

	else if (is_trading_menu_open())
		return recog.is_button(recog.get_pane(trading_params::pane_menu_execute),
			trading_params::background_marine_blue,
			trading_params::background_grey_dark);

	return false;
}

bool trading_menu::can_buy(unsigned int index) const
{
	if(!is_trading_menu_open())
		return false;

	cv::Rect2i offering_loc = get_abs_location(index);
	cv::Mat icon_img =image_recognition::get_pane(trading_params::size_icon, recog.screenshot(offering_loc));

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/icon.png", icon_img);
#endif

	return recog.closer_to(icon_img.at<cv::Vec4b>(0, icon_img.cols / 2), trading_params::frame_brown, trading_params::background_grey_bright);
}

std::vector<offering> trading_menu::get_offerings() const
{
	if(!is_trade_preview_open())
		return std::vector<offering>();

	std::vector<offering> result;

	int price_width = trading_params::size_price.width;
	int price_height = trading_params::size_price.height;

	cv::imwrite("debug_images/offerings.png", recog.get_pane(trading_params::pane_prev_offering));

	for (unsigned int index = 0; index < 8; index++)
	{
		
		cv::Rect2i offering_loc = get_abs_location(index);
		cv::Mat price_img = recog.binarize(image_recognition::get_pane(trading_params::size_price, recog.screenshot(offering_loc)), true);

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
		cv::imwrite("debug_images/offering.png", recog.screenshot(offering_loc));
		cv::imwrite("debug_images/price.png", price_img);
#endif

		int price = recog.number_from_region(price_img);

		if (price > 0)
			result.push_back({ index, static_cast<unsigned int>(price) });
	}

	return result;
}

cv::Rect2i trading_menu::get_abs_location(unsigned int index) const
{
	cv::Rect2f box(get_rel_location(index));

	int width = recog.get_screenshot().cols;
	int height = recog.get_screenshot().rows;

	return cv::Rect2i(box.x * width, box.y * height, box.width * width, box.height * height);
}

cv::Rect2f trading_menu::get_rel_location(unsigned int index) const
{
	if (!is_trade_preview_open() && !is_trading_menu_open())
		throw std::runtime_error("No menu open");

	cv::Rect2f area = is_trade_preview_open() ? trading_params::pane_prev_offering : trading_params::pane_menu_offering;
	float col_total_margin = area.width - trading_params::count_cols * trading_params::size_offering.width;
	float col_margin = col_total_margin / (trading_params::count_cols - 1);

	float row_total_margin = area.height - trading_params::count_rows * trading_params::size_offering.height;
	float row_margin = row_total_margin / (trading_params::count_rows - 1);

	int col = index % trading_params::count_cols;
	int row = index / trading_params::count_cols;

	return cv::Rect2f(col * (col_margin + trading_params::size_offering.width) + area.x,
		row * (row_margin + trading_params::size_offering.height) + area.y,
		trading_params::size_offering.width,
		trading_params::size_offering.height);
}

unsigned int trading_menu::get_open_trader() const
{
	return open_trader;
}

bool trading_menu::is_book(unsigned int index) const
{
	cv::Mat offering(recog.get_pane(get_rel_location(index)));
	cv::Vec4b pixel = offering.at<cv::Vec4b>(42, 37);
	char g = pixel(1);
	return g > pixel(0) + 10 && g > pixel(2) + 10;
}



}