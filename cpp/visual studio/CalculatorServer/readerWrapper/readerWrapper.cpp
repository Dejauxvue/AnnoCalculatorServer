#include "pch.h"

#include "readerWrapper.h"
#include "reader_util.hpp"

void reader::TradingMenu::update(const std::string& language)
{
	NativePtr->update(language, image_recognition::take_screenshot());
}

bool reader::TradingMenu::is_trade_preview_open()
{
	return NativePtr->is_trade_preview_open();
}

bool reader::TradingMenu::is_trading_menu_open()
{
	return NativePtr->is_trading_menu_open();
}

bool reader::TradingMenu::has_reroll()
{
	return NativePtr->has_reroll();
}

bool reader::TradingMenu::can_buy()
{
	return NativePtr->can_buy();
}

bool reader::TradingMenu::can_buy(unsigned int index)
{
	return NativePtr->can_buy(index);
}

bool reader::TradingMenu::can_buy(const offering& off)
{
	return NativePtr->can_buy(off);
}

std::vector<reader::offering> reader::TradingMenu::get_offerings()
{
	return NativePtr->get_offerings();
}

std::vector<reader::offering> reader::TradingMenu::get_capped_items()
{
	return NativePtr->get_capped_items();
}

int reader::TradingMenu::get_price_modification()
{
	return NativePtr->get_price_modification();
}

cv::Rect2i reader::TradingMenu::get_abs_location(unsigned int index)
{
	return NativePtr->get_abs_location(index);
}

cv::Rect2f reader::TradingMenu::get_rel_location(unsigned int index)
{
	return NativePtr->get_rel_location(index);
}

unsigned int reader::TradingMenu::get_open_trader()
{
	return NativePtr->get_open_trader();
}

reader::ImageRecognition::ImageRecognition() :
	NativePtr(new image_recognition())
{
}

Test::Test()
{
	throw gcnew System::NotImplementedException();
}
