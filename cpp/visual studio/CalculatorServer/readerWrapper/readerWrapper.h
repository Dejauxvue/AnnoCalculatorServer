#pragma once

#include "reader_trading.hpp"

using namespace System;

public ref class Test {
public:
	Test();
};

namespace reader {
	public ref class ImageRecognition
	{
		image_recognition* NativePtr;

	public:
		ImageRecognition();
		~ImageRecognition() { delete NativePtr; }

		image_recognition* get_native() { return NativePtr; }
	};

	public ref class TradingMenu
	{
		trading_menu* NativePtr;

	public:
		TradingMenu(ImageRecognition^ recog) : NativePtr(new trading_menu(*recog->get_native())) {}
		~TradingMenu() { delete NativePtr; }

		void update(const std::string& language);


		bool is_trade_preview_open();
		bool is_trading_menu_open();
		bool has_reroll();
		/*
		* Trading menue: Tests wheter the trade button can be clicked
		* Preview: Tests whether the open trade menu button is available
		*/
		bool can_buy();
		/*
		 * Trading menue: Tests wheter the item at the specified index can be added to the cart
		 * Returns false in all other cases
		 */
		bool can_buy(unsigned int index);
		bool can_buy(const offering& off);

		/*
		* Returns all currently offered items
		*/
		std::vector<offering> get_offerings();
		std::vector<offering> get_capped_items();

		/**
		* Returns the number of (total slots, empty slots)
		*/
		std::pair<unsigned int, unsigned int> get_cargo_slot_count();

		/*
		* Returns the modification of trading prices introduced
		* by capped itemes
		*/
		int get_price_modification();

		/**
		* Given an index from an @ref{offering} it returns
		* the position of the corresponding item on the screenshot.
		* Distinguishes whether preview or trade menu is open.
		*/
		cv::Rect2i get_abs_location(unsigned int index);
		cv::Rect2f get_rel_location(unsigned int index);

		unsigned int get_open_trader();

	};
}
