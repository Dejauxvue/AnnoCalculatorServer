#include "reader_trading.hpp"

#include <iostream>

#include <boost/algorithm/string.hpp>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

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
const cv::Scalar trading_params::background_cargo_slot = cv::Scalar(107, 148, 172, 255);
const cv::Scalar trading_params::background_trading_menu = cv::Scalar(122, 163, 188, 255);
const cv::Scalar trading_params::frame_brown = cv::Scalar(89, 152, 195, 255);
const cv::Scalar trading_params::red_icon = cv::Scalar(22, 42, 189, 255);

const cv::Rect2f trading_params::size_offering_icon = cv::Rect2f(cv::Point2f(0.0185, 0.23), cv::Point2f(1., 1.));
const cv::Rect2f trading_params::size_offering_price = cv::Rect2f(cv::Point2f(0.218, 0.015), cv::Point2f(1., 0.19));
const cv::Rect2f trading_params::size_offering = cv::Rect2f(0, 0, 0.035780, 0.080314);
const cv::Rect2f trading_params::size_icon = cv::Rect2f(0, 0, 0.040453, 0.071168);
const cv::Rect2f trading_params::size_icon_small = cv::Rect2f(0, 0, 0.031853, 0.056719);

const unsigned int trading_params::count_cols = 4;
const unsigned int trading_params::count_rows = 3;

const float trading_params::off_x = 0.08569;
const float trading_params::off_y = 0.05502;

const cv::Rect2f trading_params::pane_menu_offering = cv::Rect2f(cv::Point2f(trading_params::off_x + 0.62303, trading_params::off_y + 0.42088), cv::Point2f(trading_params::off_x + 0.78273, trading_params::off_y + 0.67813));
const cv::Rect2f trading_params::pane_menu_offering_with_counter = cv::Rect2f(cv::Point2f(trading_params::off_x + 0.62303, trading_params::off_y + 0.46742), cv::Point2f(trading_params::off_x + 0.78273, trading_params::off_y + 0.72466));
const cv::Rect2f trading_params::pane_menu_name = cv::Rect2f(cv::Point2f(trading_params::off_x + 0.65148, trading_params::off_y + 0.34788), cv::Point2f(trading_params::off_x + 0.74859, trading_params::off_y + 0.37243));
const cv::Rect2f trading_params::pane_menu_reroll = cv::Rect2f(cv::Point2f(trading_params::off_x + 0.68899, trading_params::off_y + 0.71265), cv::Point2f(trading_params::off_x + 0.71897, trading_params::off_y + 0.76670));
const cv::Rect2f trading_params::pane_menu_execute = cv::Rect2f(cv::Point2f(trading_params::off_x + 0.4397, trading_params::off_y + 0.7793), cv::Point2f(trading_params::off_x + 0.5616, trading_params::off_y + 0.8081));
const cv::Rect2f trading_params::pane_menu_title = cv::Rect2f(cv::Point2f(trading_params::off_x + 0.45904, trading_params::off_y + 0.19417), cv::Point2f(trading_params::off_x + 0.53951, trading_params::off_y + 0.23381));
const cv::Rect2f trading_params::pane_menu_ship_cargo = cv::Rect2f(cv::Point2f(trading_params::off_x + 0.22050, trading_params::off_y + 0.43526), cv::Point2f(trading_params::off_x + 0.37741, trading_params::off_y + 0.63224));
const cv::Rect2f trading_params::pane_menu_ship_sockets = cv::Rect2f(cv::Point2f(trading_params::off_x + 0.22019, trading_params::off_y + 0.51098), cv::Point2f(trading_params::off_x + 0.37766, trading_params::off_y + 0.73094));
const cv::Rect2f trading_params::pane_menu_available_items = cv::Rect2f(cv::Point2f(trading_params::off_x + 0.65217, trading_params::off_y + 0.41544), cv::Point2f(trading_params::off_x + 0.75351, trading_params::off_y + 0.43787));

const cv::Rect2f trading_params::pane_tooltip_reroll_heading = cv::Rect2f(cv::Point2f(trading_params::off_x + 0.7238, trading_params::off_y + 0.776), cv::Point2f(trading_params::off_x + 0.8352, trading_params::off_y + 0.8044));
const cv::Rect2f trading_params::pane_tooltip_reroll_price = cv::Rect2f(cv::Point2f(trading_params::off_x + 0.8, trading_params::off_y + 0.8412), cv::Point2f(trading_params::off_x + 0.8434, trading_params::off_y + 0.8607));

const cv::Point2f trading_params::pixel_ship_full = cv::Point2f(trading_params::off_x + 0.2375, trading_params::off_y + 0.4382);
const cv::Point2f trading_params::pixel_background_sockets_color = cv::Point2f(trading_params::off_x + 0.32536, trading_params::off_y + 0.415692);

////////////////////////////////////////
//
// Class: trading_menu
//
////////////////////////////////////////

bool offering::operator==(const offering& other) const
{
	return index == other.index && box == other.box && price == other.price && item_candidates == other.item_candidates;
}

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

	recog.crop_widescreen(img).copyTo(this->screenshot);

	if (recog.is_verbose()) {
		cv::imwrite("debug_images/screenshot_cropped.png", this->screenshot);
	}

	window_width = img.cols;
	recog.update(language);


	// test if trading menu is open
	cv::Mat trading_menu_title = recog.binarize(recog.get_pane(trading_params::pane_menu_title, screenshot), true);

	if (recog.is_verbose()) {
		cv::imwrite("debug_images/trading_menu_title.png", trading_menu_title);
	}

	menu_open = !recog.get_guid_from_name(trading_menu_title, recog.make_dictionary({ phrase::TRADE })).empty();

	if (menu_open)
	{
		cv::Mat trader_name = recog.binarize(recog.get_pane(trading_params::pane_menu_name, screenshot), true);
		if (recog.is_verbose()) {
			cv::imwrite("debug_images/trader_name.png", trader_name);
		}

		auto trader_candidates = recog.get_guid_from_name(trader_name, recog.get_dictionary().traders);

		if (!trader_candidates.empty())
			open_trader = trader_candidates.front();

		cv::Mat img_buy_limit = recog.binarize(recog.get_pane(trading_params::pane_menu_available_items, screenshot), false);
		if (recog.is_verbose()) {
			cv::imwrite("debug_images/buy_limit.png", img_buy_limit);
		}


		std::vector<std::pair<std::string, cv::Rect>> words = recog.detect_words(img_buy_limit, tesseract::PSM_SINGLE_LINE);
		std::string number_string;

		std::string buy_limit_text = recog.join(words, true);
		std::vector<std::string> split_string;
		boost::split(split_string, buy_limit_text, [](char c) {return c == ':' || c >= '0' && c <= '9'; });

		buy_limited = !recog.get_guid_from_name(split_string.front(), recog.make_dictionary({ phrase::AVAILABE_ITEMS, phrase::PURCHASABLE_ITEMS })).empty();
		if (buy_limited)
			buy_limit = recog.number_from_string(buy_limit_text.substr(split_string.front().size()));
		else
			buy_limit = std::numeric_limits<unsigned int>::max();

	}


}

bool trading_menu::is_trading_menu_open() const
{
	return menu_open;
}

bool trading_menu::has_reroll() const
{
	if (is_trading_menu_open())
		return recog.is_button(recog.get_pane(trading_params::pane_menu_reroll, screenshot),
			trading_params::background_marine_blue,
			trading_params::background_sand_bright);

	return false;
}

bool trading_menu::has_buy_limit() const
{
	return buy_limited;
}

bool trading_menu::can_buy() const
{
	if (is_trading_menu_open())
		return recog.is_button(recog.get_pane(trading_params::pane_menu_execute, screenshot),
			trading_params::background_marine_blue,
			trading_params::background_grey_dark);

	return false;
}

bool trading_menu::can_buy(unsigned int index) const
{
	if (!is_trading_menu_open())
		return false;

	cv::Rect2i offering_loc = get_roi_abs_location(index);
	cv::Mat icon_img = image_recognition::get_pane(trading_params::size_offering_icon, screenshot(offering_loc));

	if (recog.is_verbose()) {
		cv::imwrite("debug_images/icon.png", icon_img);
	}

	return recog.closer_to(icon_img.at<cv::Vec4b>(0, icon_img.cols / 2), trading_params::frame_brown, trading_params::background_grey_bright);
}

bool trading_menu::can_buy(const offering& off) const
{
	cv::Mat gray_icon;
	const auto& item = *off.item_candidates.front();
	cv::cvtColor(item.icon, gray_icon, cv::COLOR_BGRA2GRAY);
	cv::cvtColor(gray_icon, gray_icon, cv::COLOR_GRAY2BGRA); // restore the 4 channels

	const auto result = recog.get_guid_from_icon(
		screenshot(off.box),
		{
			{item.guid, item.icon},
			{0, gray_icon}
		},
		trading_params::background_sand_bright
	);

	return !result.empty() && result.front() == item.guid;
}

bool trading_menu::is_ship_full() const
{
	cv::Vec4b pixel = screenshot.at<cv::Vec4b>(static_cast<int>(trading_params::pixel_ship_full.y * screenshot.rows),
		static_cast<int>(trading_params::pixel_ship_full.x * screenshot.cols));
	return image_recognition::closer_to(pixel, trading_params::red_icon, trading_params::background_trading_menu);
}


int trading_menu::get_price(const cv::Mat& offering)
{
	cv::Mat price_img = recog.binarize(image_recognition::get_pane(trading_params::size_offering_price, offering), true, false);
	unsigned int count_black = price_img.rows * price_img.cols - cv::countNonZero(price_img);

	if (recog.is_verbose()) {
		cv::imwrite("debug_images/price.png", price_img);
	}
	
	auto iter = cached_prices.find(count_black);
	int price = 0;

	if(iter != cached_prices.end())
	{
		for(const auto& rendered_price : iter->second)
		{
			const auto& cached_img = rendered_price.second;
			if (cached_img.rows != price_img.rows || cached_img.cols != price_img.cols)
				continue;

			cv::Mat diff;
			
			cv::bitwise_xor(price_img, cached_img, diff);

			if(!cv::countNonZero(diff))
			{
				price = rendered_price.first;
				//std::cout << "price cache hit: " << rendered_price.first << std::endl;
				break;
			}
		}
	}

	if (!price)
	{
		cv::Mat price_img_rgb;
		cv::cvtColor(price_img, price_img_rgb, cv::COLOR_GRAY2RGBA);
		price = recog.number_from_region(price_img_rgb);

		if(iter == cached_prices.end())
		{
			iter = cached_prices.emplace(count_black, std::vector<std::pair<int, cv::Mat>>()).first;
		} 
		iter->second.emplace_back(price, price_img);
	}

	return price;
}

bool trading_menu::check_price(unsigned int guid, unsigned int selling_price, int price_modification_percent) const
{
	float multiplier = 1.f + price_modification_percent / 100.f;
	float price = multiplier * recog.items[guid]->price;
	return std::floor(price - 0.5f) <= selling_price && selling_price <= std::ceil(price + 0.5f);
}

std::vector<offering> trading_menu::get_offerings(bool abort_if_not_loaded)
{
	if (!is_trading_menu_open() || !open_trader)
		return std::vector<offering>();

	std::vector<offering> result;

	cv::Rect2f offering_pane = has_buy_limit() ? trading_params::pane_menu_offering_with_counter : trading_params::pane_menu_offering;
	cv::Mat pane;
	recog.get_pane(offering_pane, screenshot).copyTo(pane);

	cv::Point2f button_offset = reader::trading_params::pane_menu_reroll.tl() - offering_pane.tl();
	cv::Rect2i button_reroll(static_cast<int>(button_offset.x * screenshot.cols),
		static_cast<int>(button_offset.y * screenshot.rows),
		static_cast<int>(reader::trading_params::pane_menu_reroll.width * screenshot.cols),
		static_cast<int>(reader::trading_params::pane_menu_reroll.height * screenshot.rows));

	if (recog.is_verbose()) {
		cv::imwrite("debug_images/offerings.png", pane);
	}

	cv::Rect2i offering_size = get_roi_abs_location(0);

	std::vector<cv::Rect2i> boxes(recog.detect_boxes(pane, offering_size, button_reroll));

	std::sort(boxes.begin(), boxes.end(), [&offering_size](const cv::Rect2i& lhs, const cv::Rect2i& rhs) {
		if (lhs.y + offering_size.height < rhs.y)
			return true;
		else if (rhs.y + offering_size.height < lhs.y)
			return false;
		else if (lhs.x + offering_size.width < rhs.x)
			return true;
		return false;
		});

	unsigned int index = 0;
	int trade_price_modifier = get_price_modification();

	for (const cv::Rect2i& offering_loc : boxes)
	{
		int price = get_price(pane(offering_loc));
		std::map<unsigned int, cv::Mat> icon_dictionary;


		for (unsigned int guid : recog.trader_to_offerings.at(open_trader))
		{
			if (recog.items.find(guid) == recog.items.end())
				continue;

			if (check_price(guid, price, trade_price_modifier))
				icon_dictionary.emplace(guid, recog.items[guid]->icon);
		}

		std::vector<unsigned int> item_candidates;

		if (icon_dictionary.size() == 1)
			item_candidates.push_back(icon_dictionary.begin()->first);
		else
		{
			if (icon_dictionary.empty())
			{
				for (unsigned int guid : recog.trader_to_offerings.at(open_trader))
				{
					if (recog.items.find(guid) == recog.items.end())
						continue;
					icon_dictionary.emplace(guid, recog.items[guid]->icon);
				}
			}

			for (const auto& entry : recog.item_backgrounds)
				icon_dictionary.insert(entry);

			item_candidates = recog.get_guid_from_icon(
				image_recognition::get_pane(trading_params::size_offering_icon, pane(offering_loc)),
				icon_dictionary,
				trading_params::background_sand_bright
			);
		}

		if (abort_if_not_loaded &&
			(!item_candidates.size() ||
				recog.item_backgrounds.find(item_candidates.front()) != recog.item_backgrounds.end()))
		{
			result.clear();
			return result;
		}

		if (recog.is_verbose()) {
			cv::imwrite("debug_images/offering.png", pane(offering_loc));
		}



		if (!item_candidates.empty())
		{
			std::vector<item::ptr> items;
			for (unsigned int guid : item_candidates)
				items.push_back(recog.items[guid]);

			cv::Rect2i abs_box(
				static_cast<int>(screenshot.cols * offering_pane.x + offering_loc.x),
					static_cast<int>(screenshot.rows * offering_pane.y + offering_loc.y),
				offering_loc.width,
				offering_loc.height
			);

			result.emplace_back(offering{
				index++,
				get_window_abs_location(abs_box),
				(unsigned int)price,
				std::move(items)
				});
		}

	}



	return result;
}

std::vector<offering> trading_menu::get_capped_items() const
{
	std::vector<offering> result;
	
	if (!is_trading_menu_open())
		return result;

	if (recog.is_verbose()) {
		std::cout << "equipped items: ";
		cv::imwrite("debug_images/item_sockets.png", recog.get_pane(trading_params::pane_menu_ship_sockets, screenshot));
	}

	cv::Rect2i icon_size(0, 0, static_cast<int>(trading_params::size_icon.width * screenshot.cols), static_cast<int>(trading_params::size_icon.height * screenshot.rows));
	cv::Mat pane(recog.get_pane(trading_params::pane_menu_ship_sockets, screenshot));
	std::vector<cv::Rect2i> boxes(recog.detect_boxes(pane, icon_size));

	if (boxes.size() <= 1)
	{
		cv::Rect2i icon_size_small(0, 0, static_cast<int>(trading_params::size_icon_small.width * screenshot.cols), static_cast<int>(trading_params::size_icon_small.height * screenshot.rows));
		boxes = recog.detect_boxes(pane, icon_size_small);

		if (boxes.empty())
			return result;
	}

	std::sort(boxes.begin(), boxes.end(), [&icon_size](const cv::Rect2i& lhs, const cv::Rect2i& rhs) {
		if (lhs.y > rhs.y + icon_size.height)
			return true;
		else if (rhs.y > lhs.y + icon_size.height)
			return false;
		else if (lhs.x + icon_size.width < rhs.x)
			return true;
		return false;
		});

	unsigned int index = 0;
	int row = boxes.front().y;

	for (const cv::Rect2i& item_loc : boxes)
	{
		if (item_loc.y + item_loc.height < row)
			break;

		std::vector<unsigned int> item_candidates = recog.get_guid_from_icon(
			pane(item_loc),
			ship_items,
			trading_params::background_cargo_slot
		);

		if (recog.is_verbose()) {
			cv::imwrite("debug_images/item.png", pane(item_loc));
		}



		if (!item_candidates.empty())
		{
			std::vector<item::ptr> items;
			for (unsigned int guid : item_candidates)
				items.push_back(recog.items[guid]);

			cv::Rect2i abs_box(
				static_cast<int>(screenshot.cols * trading_params::pane_menu_ship_sockets.x + item_loc.x),
					static_cast<int>(screenshot.rows * trading_params::pane_menu_ship_sockets.y + item_loc.y),
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

	std::cout << std::endl;

	return result;
}

cv::Rect2f trading_menu::get_execute_button() const
{
	return get_window_rel_location(reader::trading_params::pane_menu_execute);
}

cv::Rect2f trading_menu::get_reroll_button() const
{
	return get_window_rel_location(reader::trading_params::pane_menu_reroll);
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



cv::Rect2i trading_menu::get_window_abs_location(unsigned int index) const
{
	return get_window_abs_location(get_roi_abs_location(index));
}

cv::Rect2f trading_menu::get_window_rel_location(unsigned int index) const
{
	return get_window_rel_location(get_roi_rel_location(index));
}

cv::Rect2i trading_menu::get_window_abs_location(const cv::Rect2i& roi_abs_location) const
{
	if (window_width == screenshot.cols)
		return roi_abs_location;

	const cv::Rect2f& box = roi_abs_location;

	return cv::Rect2i(box.x + (window_width - screenshot.cols) / 2.f,
		box.y,
		box.width,
		box.height);
}

cv::Rect2f trading_menu::get_window_rel_location(const cv::Rect2f& roi_rel_location) const
{
	if (window_width == screenshot.cols)
		return roi_rel_location;

	const cv::Rect2f& box = roi_rel_location;

	return cv::Rect2f((box.x - 0.5f) * screenshot.cols / window_width + 0.5f,
		box.y,
		box.width * screenshot.cols / window_width,
		box.height);
}

unsigned int trading_menu::get_open_trader() const
{
	return open_trader;
}

unsigned int trading_menu::get_reroll_cost() const
{
	if (!is_trading_menu_open())
		return 0;

	cv::Mat tooltip_heading = recog.binarize(recog.get_pane(trading_params::pane_tooltip_reroll_heading, screenshot), true);
	if (recog.is_verbose()) {
		cv::imwrite("debug_images/tooltip_heading.png", tooltip_heading);
	}
	
	if (recog.get_guid_from_name(tooltip_heading, recog.make_dictionary({ phrase::REROLL_OFFERED_ITEMS })).empty())
		return 0;

	cv::Mat reroll_costs = recog.binarize(recog.get_pane(trading_params::pane_tooltip_reroll_price, screenshot), true);
	if (recog.is_verbose()) {
		cv::imwrite("debug_images/reroll_costs.png", reroll_costs);
	}
	return recog.number_from_region(reroll_costs);
}

unsigned int trading_menu::get_buy_limit() const
{
	if (!has_buy_limit())
		return std::numeric_limits<unsigned int>::max();

	return buy_limit;
}

cv::Rect2i trading_menu::get_roi_abs_location(unsigned int index) const
{
	return to_abs_location(get_roi_rel_location(index));
}

cv::Rect2i trading_menu::to_abs_location(const cv::Rect2f& box) const
{
	int width = screenshot.cols;
	int height = screenshot.rows;

	return cv::Rect2i(box.x * width, box.y * height, box.width * width, box.height * height);
}

cv::Rect2f trading_menu::get_roi_rel_location(unsigned int index) const
{
	if (!is_trading_menu_open())
		throw std::runtime_error("No menu open");

	cv::Rect2f area = has_buy_limit() ? trading_params::pane_menu_offering_with_counter : trading_params::pane_menu_offering;
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

}