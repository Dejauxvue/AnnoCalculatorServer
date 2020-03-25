#include "reader_trading.hpp"

#include "reader_util.hpp"

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
const cv::Scalar trading_params::background_cargo_slot = cv::Scalar(108,147,175, 255);
const cv::Scalar trading_params::background_trading_menu = cv::Scalar(122, 163, 188, 255);
const cv::Scalar trading_params::frame_brown = cv::Scalar(89, 152, 195, 255);

const cv::Rect2f trading_params::size_offering_icon = cv::Rect2f(cv::Point2f(0.0185, 0.23), cv::Point2f(1.,1.));
const cv::Rect2f trading_params::size_offering_price = cv::Rect2f(cv::Point2f(0.218, 0.), cv::Point2f(1., 0.225));
const cv::Rect2f trading_params::size_offering = cv::Rect2f(0,0,0.0390625,0.08796);
const cv::Rect2f trading_params::size_icon = cv::Rect2f(0, 0, 0.040625, 0.072222);

const unsigned int trading_params::count_cols = 4;
const unsigned int trading_params::count_rows = 2;

const cv::Rect2f trading_params::pane_prev_name = cv::Rect2f(cv::Point2f(0.8386, 0.5676), cv::Point2f(0.974, 0.5942));
const cv::Rect2f trading_params::pane_prev_tab_items = cv::Rect2f(cv::Point2f(0.9148, 0.62), cv::Point2f(0.9827, 0.6462));
const cv::Rect2f trading_params::pane_prev_offering = cv::Rect2f(cv::Point2f(0.8314, 0.661), cv::Point2f(0.9859, 0.8241));
const cv::Rect2f trading_params::pane_prev_reroll = cv::Rect2f(cv::Point2f(0.8954, 0.8354), cv::Point2f(0.924, 0.8864));
const cv::Rect2f trading_params::pane_prev_exchange = cv::Rect2f(cv::Point2f(0.8955, 0.9352), cv::Point2f(0.9241, 0.9862));

const cv::Rect2f trading_params::pane_menu_offering = cv::Rect2f(cv::Point2f(0.6238, 0.4217), cv::Point2f(0.7791, 0.7061));
const cv::Rect2f trading_params::pane_menu_name = cv::Rect2f(cv::Point2f(0.6354, 0.3477), cv::Point2f(0.766, 0.3717));
const cv::Rect2f trading_params::pane_menu_reroll = cv::Rect2f(cv::Point2f(0.6897, 0.7149), cv::Point2f(0.7183, 0.7658));
const cv::Rect2f trading_params::pane_menu_execute = cv::Rect2f(cv::Point2f(0.4397, 0.7793), cv::Point2f(0.5616, 0.8081));
const cv::Rect2f trading_params::pane_menu_title = cv::Rect2f(cv::Point2f(0.4356, 0.1937), cv::Point2f(0.5704, 0.2335));
const cv::Rect2f trading_params::pane_menu_ship_cargo = cv::Rect2f(cv::Point2f(0.2272, 0.4264), cv::Point2f(0.3715, 0.5977));
const cv::Rect2f trading_params::pane_menu_ship_sockets = cv::Rect2f(cv::Point2f(0.2272, 0.6207), cv::Point2f(0.3715, 0.701));


////////////////////////////////////////
//
// Class: trading_menu
//
////////////////////////////////////////

trading_menu::trading_menu(image_recognition& recog)
	:
	recog(recog),
	storage_icon(recog.binarize_icon(image_recognition::load_image("icons/icon_goods_storage.png"))),
	open_trader(0),
	menu_open(false)
{
	for (const auto& item : recog.items)
		if (item.second->isShipAllocation())
			ship_items.emplace(item.first, item.second->icon);
}

void trading_menu::update(const std::string& language, const cv::Mat& img)
{
	open_trader = 0;
	menu_open = false;

	if (!img.cols)
		return;

	recog.update(language, img);


		// test if trading menu is open
		cv::Mat trading_menu_title = recog.binarize(recog.get_pane(trading_params::pane_menu_title), true);

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
		cv::imwrite("debug_images/trading_menu_title.png", trading_menu_title);
#endif

		menu_open = !recog.get_guid_from_name(trading_menu_title, recog.make_dictionary({ phrase::TRADE })).empty();

		if (menu_open)
		{
			cv::Mat trader_name = recog.binarize(recog.get_pane(trading_params::pane_menu_name), true);
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
			cv::imwrite("debug_images/trader_name.png", trader_name);
#endif

			auto trader_candidates = recog.get_guid_from_name(trader_name, recog.get_dictionary().traders);

			if(!trader_candidates.empty())
				open_trader = trader_candidates.front();
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
	cv::Mat icon_img =image_recognition::get_pane(trading_params::size_offering_icon, recog.screenshot(offering_loc));

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/icon.png", icon_img);
#endif

	return recog.closer_to(icon_img.at<cv::Vec4b>(0, icon_img.cols / 2), trading_params::frame_brown, trading_params::background_grey_bright);
}

bool trading_menu::can_buy(const offering& off) const
{
	cv::Mat gray_icon;
	const auto& item = *off.item_candidates.front();
	cv::cvtColor(item.icon, gray_icon, cv::COLOR_BGRA2GRAY);
	cv::cvtColor(gray_icon, gray_icon, cv::COLOR_GRAY2BGRA); // restore the 4 channels
	
	const auto result = recog.get_guid_from_icon(
		recog.screenshot(off.box),
		{
			{item.guid, item.icon},
			{0, gray_icon}
		},
		trading_params::background_sand_bright
	);

	return !result.empty() && result.front() == item.guid;
}

bool trading_menu::is_empty_cargo_slot(const cv::Mat& img) const
{
	cv::Rect2i roi(0.1f * img.cols, 0.1f * img.rows,
		0.8f * img.cols, 0.8f * img.rows);

	cv::Mat icon_processed = recog.binarize_icon(img(roi), storage_icon.size());
	int icon_white_count = cv::countNonZero(icon_processed);
	cv::bitwise_and(storage_icon, icon_processed, icon_processed);

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/icon_intersect.png", icon_processed);
#endif

	float max_intersection = std::max(icon_white_count, cv::countNonZero(storage_icon));
	float match = cv::countNonZero(icon_processed) / max_intersection;

	return match > 0.8;
}

bool trading_menu::check_price(unsigned int guid, unsigned int selling_price, int price_modification_percent) const
{
	float multiplier = 1.f + price_modification_percent / 100.f;
	float price = multiplier * recog.items[guid]->price;
	return std::floor(price) <= selling_price && selling_price <= std::ceil(price);
}

std::vector<offering> trading_menu::get_offerings() const
{
	if(!is_trading_menu_open())
		return std::vector<offering>();

	std::vector<offering> result;

	int price_width = trading_params::size_offering_price.width;
	int price_height = trading_params::size_offering_price.height;

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/offerings.png", recog.get_pane(trading_params::pane_menu_offering));
#endif

	cv::Rect2i offering_size = get_abs_location(0);
	cv::Mat pane(recog.get_pane(trading_params::pane_menu_offering));
	std::vector<cv::Rect2i> boxes(recog.detect_boxes(pane, offering_size));

	std::sort(boxes.begin(), boxes.end(), [&offering_size](const cv::Rect2i& lhs, const cv::Rect2i& rhs) {
		if (lhs.y + offering_size.height < rhs.y)
			return true;
		else if (lhs.x + offering_size.width < rhs.x)
			return true;
		return false;
		});

	unsigned int index = 0;
	int trade_price_modifier = get_price_modification();

	for (const cv::Rect2i& offering_loc : boxes)
	{
		cv::Mat price_img = recog.binarize(image_recognition::get_pane(trading_params::size_offering_price, pane(offering_loc)), true);

		int price = recog.number_from_region(price_img);
		std::map<unsigned int, cv::Mat> icon_dictionary;
		

		for (unsigned int guid : recog.trader_to_offerings.at(open_trader))
		{
			if (recog.items.find(guid) == recog.items.end())
				continue;

			if (check_price(guid, price, trade_price_modifier))
				icon_dictionary.emplace(guid, recog.items[guid]->icon);
		}

		std::vector<unsigned int> item_candidates = recog.get_guid_from_icon(
			image_recognition::get_pane(trading_params::size_offering_icon, pane(offering_loc)),
			icon_dictionary,
			trading_params::background_sand_bright
		);

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
		cv::imwrite("debug_images/offering.png", recog.screenshot(offering_loc));
		cv::imwrite("debug_images/price.png", price_img);
#endif

		
		
		if (!item_candidates.empty())
		{
			std::vector<item::ptr> items;
			for (unsigned int guid : item_candidates)
				items.push_back(recog.items[guid]);

			cv::Rect2i abs_box(
				recog.screenshot.cols * trading_params::pane_menu_offering.x + offering_loc.x,
				recog.screenshot.rows * trading_params::pane_menu_offering.y + offering_loc.y,
				offering_loc.width,
				offering_loc.height
			);

			result.emplace_back(offering{
				index++,
				abs_box,
				(unsigned int) price,
				std::move(items)
				});
		}
			
	}



	return result;
}

std::vector<offering> trading_menu::get_capped_items() const
{
	if (!is_trading_menu_open())
		return std::vector<offering>();

	std::vector<offering> result;

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/offerings.png", recog.get_pane(trading_params::pane_menu_ship_sockets));
#endif

	cv::Rect2i icon_size(0, 0, trading_params::size_icon.width * recog.screenshot.cols, trading_params::size_icon.height * recog.screenshot.rows);
	cv::Mat pane(recog.get_pane(trading_params::pane_menu_ship_sockets));
	std::vector<cv::Rect2i> boxes(recog.detect_boxes(pane, icon_size));

	std::sort(boxes.begin(), boxes.end(), [&icon_size](const cv::Rect2i& lhs, const cv::Rect2i& rhs) {
		if (lhs.y + icon_size.height < rhs.y)
			return true;
		else if (lhs.x + icon_size.width < rhs.x)
			return true;
		return false;
		});

	unsigned int index = 0;

	for (const cv::Rect2i& item_loc : boxes)
	{

		std::vector<unsigned int> item_candidates = recog.get_guid_from_icon(
			pane(item_loc),
			ship_items,
			trading_params::background_sand_bright
		);

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
		cv::imwrite("debug_images/item.png", recog.screenshot(item_loc));
#endif



		if (!item_candidates.empty())
		{
			std::vector<item::ptr> items;
			for (unsigned int guid : item_candidates)
				items.push_back(recog.items[guid]);

			cv::Rect2i abs_box(
				recog.screenshot.cols * trading_params::pane_menu_offering.x + item_loc.x,
				recog.screenshot.rows * trading_params::pane_menu_offering.y + item_loc.y,
				item_loc.width,
				item_loc.height
			);

			result.emplace_back(offering{
				index++,
				abs_box,
				items.front()->price,
				std::move(items)
				});
		}

	}



	return result;
}

std::pair<unsigned int, unsigned int> trading_menu::get_cargo_slot_count() const
{
	if (!is_trading_menu_open())
		return std::make_pair(0,0);

	std::vector<offering> result;

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/cargo_slots.png", recog.get_pane(trading_params::pane_menu_ship_sockets));
#endif

	cv::Rect2i icon_size(0, 0, trading_params::size_icon.width * recog.screenshot.cols, trading_params::size_icon.height * recog.screenshot.rows);
	cv::Mat pane(recog.get_pane(trading_params::pane_menu_ship_cargo));
	std::vector<cv::Rect2i> boxes(recog.detect_boxes(pane, icon_size,0.05f, 50,100));

	unsigned int full_count = 0;

	for (const cv::Rect2i& item_loc : boxes)
	{
		
		if (!is_empty_cargo_slot(pane(item_loc)))
		{
			full_count++;
		}

	}



	return std::make_pair(full_count, boxes.size());
}

int trading_menu::get_price_modification() const
{
	int result = 0;
	for (const auto& item : get_capped_items())
	{
		result += item.item_candidates.front()->trade_price_modifier;
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


}