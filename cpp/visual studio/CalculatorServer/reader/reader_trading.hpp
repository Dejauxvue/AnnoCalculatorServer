#pragma once

#include "reader_util.hpp"

namespace reader
{

class trading_params
{
public:
	static const cv::Scalar background_marine_blue;
	static const cv::Scalar background_sand_dark;
	static const cv::Scalar background_sand_bright;

	static const cv::Rect2f size_price;
	static const cv::Rect2f size_icon;

	static const cv::Rect2f pane_prev_name;
	static const cv::Rect2f pane_prev_tab_items;
	static const cv::Rect2f pane_prev_offering;
	static const cv::Rect2f pane_prev_reroll;
	static const cv::Rect2f pane_prev_exchange;

	static const cv::Rect2f pane_menu_offering;
	static const cv::Rect2f pane_menu_name;
	static const cv::Rect2f pane_menu_reroll;
	static const cv::Rect2f pane_menu_title;

};

struct offering
{
	unsigned int index;
	unsigned int price;
};

class trading_menu
{
public:
	trading_menu(image_recognition& recog);

	void update(const std::string& language, const cv::Mat& img);


	bool is_trade_preview_open() const;
	bool is_trading_menu_open() const;
	bool has_reroll() const;
	/*
	* Trading menue: Tests wheter the trade button can be clicked
	* Preview: Tests whether the open trade menu button is available
	*/
	bool can_buy() const;

	/*
	* Returns all currently offered items
	*/
	std::vector<offering> get_offerings() const;

	/**
	* Given an index from an @ref{offering} it returns 
	* the position of the corresponding item on the screenshot.
	* Distinguishes whether preview or trade menu is open.
	*/
	cv::Rect2i get_location(unsigned int index);

private:
	image_recognition& recog;

	unsigned int open_trader;
	bool menu_open;

};

}