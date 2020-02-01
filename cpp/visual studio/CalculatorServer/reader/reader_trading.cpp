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

const cv::Rect2f trading_params::size_icon = cv::Rect2f(cv::Point2f(0.9495, 0.6767), cv::Point2f(0.9848,0.7396));
const cv::Rect2f trading_params::size_price = cv::Rect2f(cv::Point2f(0.9568, 0.6604), cv::Point2f(0.9846, 0.6767));

const cv::Rect2f trading_params::pane_prev_name = cv::Rect2f(cv::Point2f(0.8386, 0.5676), cv::Point2f(0.974, 0.5942));
const cv::Rect2f trading_params::pane_prev_tab_items = cv::Rect2f(cv::Point2f(0.9148, 0.62), cv::Point2f(0.9827, 0.6462));
const cv::Rect2f trading_params::pane_prev_offering = cv::Rect2f(cv::Point2f(0.8306, 0.66), cv::Point2f(0.9849, 0.8241));
const cv::Rect2f trading_params::pane_prev_reroll = cv::Rect2f(cv::Point2f(0.8954, 0.8354), cv::Point2f(0.924, 0.8864));
const cv::Rect2f trading_params::pane_prev_exchange = cv::Rect2f(cv::Point2f(0.8955, 0.9352), cv::Point2f(0.9241, 0.9862));

const cv::Rect2f trading_params::pane_menu_offering = cv::Rect2f(cv::Point2f(0.6238, 0.4293), cv::Point2f(0.7781, 0.5934));
const cv::Rect2f trading_params::pane_menu_name = cv::Rect2f(cv::Point2f(0.6354, 0.3477), cv::Point2f(0.766, 0.3717));
const cv::Rect2f trading_params::pane_menu_reroll = cv::Rect2f(cv::Point2f(0.6897, 0.7149), cv::Point2f(0.7183, 0.7658));
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
		phrase::INUIT
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

		menu_open = !recog.get_guid_from_name(trader_name, recog.make_dictionary({ phrase::TRADE })).empty();

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
		}
	}

}



}