#include "reader_statistics_screen.hpp"

#include <iostream>
#include <numeric>
#include <regex>

#include <boost/algorithm/string.hpp>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

namespace reader
{
////////////////////////////////////////
//
// Class: statistics_screen_params
//
////////////////////////////////////////

const cv::Scalar statistics_screen_params::background_blue_dark = cv::Scalar(103, 87, 79, 255);
const cv::Scalar statistics_screen_params::background_brown_light = cv::Scalar(124, 181, 213, 255);
const cv::Scalar statistics_screen_params::foreground_brown_light = cv::Scalar(96, 195, 255, 255);
const cv::Scalar statistics_screen_params::foreground_brown_dark = cv::Scalar(19, 53, 81, 255);
const cv::Scalar statistics_screen_params::expansion_arrow = cv::Scalar(78, 98, 115, 255);

const cv::Rect2f statistics_screen_params::pane_tabs = cv::Rect2f(cv::Point2f(0.2352f, 0.147f), cv::Point2f(0.7648f, 0.1839f));
const cv::Rect2f statistics_screen_params::pane_title = cv::Rect2f(cv::Point2f(0.3839f, 0), cv::Point2f(0.6176f, 0.0722f));
const cv::Rect2f statistics_screen_params::pane_all_islands = cv::Rect2f(cv::Point2f(0.0234f, 0.2658f), cv::Point2f(0.19f, 0.315f));
const cv::Rect2f statistics_screen_params::pane_islands = cv::Rect2f(cv::Point2f(0.0215f, 0.3326f), cv::Point2f(0.1917f, 0.9586f));
const cv::Rect2f statistics_screen_params::pane_finance_center = cv::Rect2f(cv::Point2f(0.2471f, 0.3084f), cv::Point2f(0.5805f, 0.9576f));
const cv::Rect2f statistics_screen_params::pane_finance_right = cv::Rect2f(cv::Point2f(0.6261f, 0.3603f), cv::Point2f(0.9635f, 0.9587f));
const cv::Rect2f statistics_screen_params::pane_production_center = cv::Rect2f(cv::Point2f(0.24802f, 0.3639f), cv::Point2f(0.583f, 0.9568f));
const cv::Rect2f statistics_screen_params::pane_production_right = cv::Rect2f(cv::Point2f(0.629f, 0.3613f), cv::Point2f(0.9619f, 0.9577f));
const cv::Rect2f statistics_screen_params::pane_population_center = cv::Rect2f(cv::Point2f(0.247f, 0.3571f), cv::Point2f(0.5802f, 0.7006f));
const cv::Rect2f statistics_screen_params::pane_header_center = cv::Rect2f(cv::Point2f(0.24485f, 0.21962f), cv::Point2f(0.4786f, 0.2581f));
const cv::Rect2f statistics_screen_params::pane_header_right = cv::Rect2f(cv::Point2f(0.6276f, 0.2238f), cv::Point2f(0.7946f, 0.2581f));

const cv::Rect2f statistics_screen_params::position_factory_icon = cv::Rect2f(0.0219f, 0.135f, 0.0838f, 0.7448f);
const cv::Rect2f statistics_screen_params::position_small_factory_icon = cv::Rect2f(cv::Point2f(0.072590f, 0.062498f), cv::Point2f(0.14436f, 0.97523f));
const cv::Rect2f statistics_screen_params::position_population_icon = cv::Rect2f(0.f, 0.05f, 0.f, 0.9f);
const cv::Rect2f statistics_screen_params::position_factory_output = cv::Rect2f(cv::Point2f(0.31963f, 0.58402f), cv::Point2f(0.49729f, 0.83325f));


////////////////////////////////////////
//
// Class: statistics_screen
//
////////////////////////////////////////

const std::string statistics_screen::KEY_AMOUNT = "amount";
const std::string statistics_screen::KEY_EXISTING_BUILDINGS = "existingBuildings";
const std::string statistics_screen::KEY_LIMIT = "limit";
const std::string statistics_screen::KEY_PRODUCTIVITY = "percentBoost";


statistics_screen::statistics_screen(image_recognition& recog)
	:
	recog(recog),
	open_tab(tab::NONE),
	center_pane_selection(0),
	selected_session(0)
{
}


void statistics_screen::update(const std::string& language, const cv::Mat& img)
{
	selected_island = std::string();
	multiple_islands_selected = false;
	selected_session = 0;
	center_pane_selection = 0;
	current_island_to_session.clear();

	recog.update(language);

	// test if open
	cv::Mat cropped_image = recog.crop_widescreen(img);

	cv::Mat statistics_text_img = recog.binarize(recog.get_pane(statistics_screen_params::pane_title, cropped_image), true);
	if (recog.is_verbose()) {
		cv::imwrite("debug_images/statistics_text.png", statistics_text_img);
		cv::imwrite("debug_images/statistics_screenshot.png", cropped_image);
	}
	if (recog.get_guid_from_name(statistics_text_img, recog.make_dictionary({ phrase::STATISTICS })).empty())
	{
		open_tab = tab::NONE;
		if (recog.is_verbose()) {
			std::cout << std::endl;
		}
		return;
	}

	if (recog.is_verbose()) {
		std::cout << std::endl;
	}

	cropped_image.copyTo(this->screenshot);

	open_tab = compute_open_tab();
	if (open_tab == tab::NONE)
		return;
	bool update = true;
	if (prev_islands.size().area() == recog.get_pane(statistics_screen_params::pane_islands, screenshot).size().area())
	{
		cv::Mat diff;
		cv::absdiff(recog.get_pane(statistics_screen_params::pane_islands, screenshot), prev_islands, diff);
		std::cout << ((float)cv::sum(diff).ddot(cv::Scalar::ones())) << std::endl;
		float match = ((float)cv::sum(diff).ddot(cv::Scalar::ones())) / prev_islands.rows / prev_islands.cols;
		update = match > 30;
	}
	if (update)
	{ // island list changed
		recog.get_pane(statistics_screen_params::pane_islands, screenshot).copyTo(prev_islands);
		update_islands();
	}
}

bool statistics_screen::is_open() const
{
	return get_open_tab() != tab::NONE;
}

statistics_screen::tab statistics_screen::get_open_tab() const
{
	return open_tab;
}

statistics_screen::tab statistics_screen::compute_open_tab() const
{
	cv::Mat tabs = recog.get_pane(statistics_screen_params::pane_tabs, screenshot);
	int tabs_count = (int)tab::ITEMS;
	int tab_width = tabs.cols / tabs_count;
	int v_center = tabs.rows / 2;
	for (int i = 1; i <= (int)tabs_count; i++)
	{
		cv::Vec4b pixel = tabs.at<cv::Vec4b>(v_center, (int)(i * tab_width - 0.2f * tab_width));
		if (is_tab_selected(pixel))
		{
			if (recog.is_verbose()) {
				cv::imwrite("debug_images/tab.png", tabs(cv::Rect(static_cast<int>(i * tab_width - 0.2f * tab_width), v_center, 10, 10)));
			}
			if (recog.is_verbose()) {
				std::cout << "Open tab:\t" << i << std::endl;
			}
			return tab(i);
		}
	}

	return tab::NONE;
}

void statistics_screen::update_islands()
{
	unsigned int session_guid = 0;

	const auto phrases = recog.make_dictionary({
		phrase::THE_OLD_WORLD,
		phrase::THE_NEW_WORLD,
		phrase::THE_ARCTIC,
		phrase::CAPE_TRELAWNEY,
		phrase::ENBESA });

	recog.iterate_rows(prev_islands, 0.75f, [&](const cv::Mat& row) {
		if (recog.is_verbose()) {
			cv::imwrite("debug_images/row.png", row);
		}

		cv::Mat subheading = recog.binarize(recog.get_cell(row, 0.01f, 0.6f, 0.f), true);

		if (recog.is_verbose()) {
			cv::imwrite("debug_images/subheading.png", subheading);
		}

		std::vector<unsigned int> ids = recog.get_guid_from_name(subheading, phrases);
		if (ids.size() == 1)
		{
			session_guid = ids.front();
			return;
		}



		if (recog.is_verbose()) {
			cv::imwrite("debug_images/selection_test.png", row(cv::Rect((int)(0.8f * row.cols), (int)(0.5f * row.rows), 10, 10)));
		}

		bool selected = is_selected(row.at<cv::Vec4b>((int)(0.5f * row.rows), (int)(0.8f * row.cols)));
		cv::Mat island_name_image = recog.binarize(recog.get_cell(row, 0.15f, 0.65f), selected);

		if (recog.is_verbose()) {
			cv::imwrite("debug_images/island_name.png", island_name_image);
		}

		std::string island_name = recog.join(recog.detect_words(island_name_image), true);
		if (island_name.empty())
			return;

		auto iter = island_to_session.find(island_name);
		if (iter != island_to_session.end())
		{
			current_island_to_session.emplace(*iter);
			return;
		}

		cv::Mat session_icon = recog.get_cell(row, 0.025f, 0.14f);
		if (recog.is_verbose()) {
			cv::imwrite("debug_images/session_icon.png", session_icon);
		}
		session_guid = recog.get_session_guid(session_icon);

		if (session_guid) {
			island_to_session.emplace(island_name, session_guid);
			current_island_to_session.emplace(island_name, session_guid);
		}

		if (recog.is_verbose()) {
			std::cout << "Island added:\t" << island_name;
			try {
				std::cout << " (" << recog.get_dictionary().ui_texts.at(session_guid) << ")";
			}
			catch (const std::exception&) {}
			std::cout << std::endl;
		}

		});
}

bool statistics_screen::is_all_islands_selected() const
{
	if (!screenshot.size || !is_open())
		return false;

	cv::Mat button = recog.get_pane(statistics_screen_params::pane_all_islands, screenshot);
	if (button.empty())
		return false;

	return is_selected(button.at<cv::Vec4b>(static_cast<int>(0.1f * button.rows), static_cast<int>(0.5f * button.cols)));
}

std::pair<std::string, unsigned int> statistics_screen::get_island_from_list(std::string name) const
{
	for (const auto& entry : island_to_session)
		if (recog.lcs_length(entry.first, name) > 0.66f * std::max(entry.first.size(), name.size()))
			return entry;

	return std::make_pair(name, 0);
}

cv::Mat statistics_screen::get_center_pane() const
{
	switch (get_open_tab())
	{
	case tab::FINANCE:
		return recog.get_pane(statistics_screen_params::pane_finance_center, screenshot);
	case tab::PRODUCTION:
		return recog.get_pane(statistics_screen_params::pane_production_center, screenshot);
	case tab::POPULATION:
		return recog.get_pane(statistics_screen_params::pane_population_center, screenshot);
	default:
		return cv::Mat();
	}
}

cv::Mat statistics_screen::get_left_pane() const
{
	switch (get_open_tab())
	{
	case tab::NONE:
		return cv::Mat();
	default:
		return recog.get_pane(statistics_screen_params::pane_islands, screenshot);
	}
}

cv::Mat statistics_screen::get_right_pane() const
{
	switch (get_open_tab())
	{
	case tab::FINANCE:
		return recog.get_pane(statistics_screen_params::pane_finance_right, screenshot);
	case tab::PRODUCTION:
		return recog.get_pane(statistics_screen_params::pane_production_right, screenshot);
	default:
		return cv::Mat();
	}
}

cv::Mat statistics_screen::get_center_header() const
{
	switch (get_open_tab())
	{
	case tab::NONE:
		return cv::Mat();
	default:
		return recog.get_pane(statistics_screen_params::pane_header_center, screenshot);
	}
}

cv::Mat statistics_screen::get_right_header() const
{
	switch (get_open_tab())
	{
	case tab::NONE:
		return cv::Mat();
	default:
		return recog.get_pane(statistics_screen_params::pane_header_right, screenshot);
	}
}

bool statistics_screen::is_selected(const cv::Vec4b& point)
{
	return image_recognition::closer_to(point, statistics_screen_params::background_blue_dark, statistics_screen_params::background_brown_light);
}

bool statistics_screen::is_tab_selected(const cv::Vec4b& point)
{
	return image_recognition::closer_to(point, cv::Scalar(234, 205, 149, 255), cv::Scalar(205, 169, 119, 255));
}








std::pair<unsigned int, int> statistics_screen::get_optimal_productivity()
{
	const cv::Mat& im = screenshot;

	if (get_open_tab() != statistics_screen::tab::PRODUCTION)
		return std::make_pair(0, 0);

	if (recog.is_verbose()) {
		std::cout << "Optimal productivities" << std::endl;
	}

	cv::Mat buildings_text = recog.binarize(im(cv::Rect(static_cast<int>(0.6522f * im.cols), static_cast<int>(0.373f * im.rows), static_cast<int>(0.093f * im.cols), static_cast<int>(0.0245f * im.rows))));
	if (recog.is_verbose()) {
		cv::imwrite("debug_images/buildings_text.png", buildings_text);
	}
	int buildings_count = recog.number_from_region(buildings_text);
	if (recog.is_verbose()) {
		std::cout << std::endl;
	}

	if (buildings_count < 0)
		return std::make_pair(0, 0);

	cv::Mat roi = get_right_pane();

	if (roi.empty())
		return std::make_pair(0, 0);

	if (recog.is_verbose()) {
		cv::imwrite("debug_images/statistics_window_scroll_area.png", roi);
	}


	cv::Mat factory_text = im(cv::Rect(static_cast<int>(0.6522f * im.cols), static_cast<int>(0.373f * im.rows), static_cast<int>(0.093f * im.cols), static_cast<int>(0.0245f * im.rows)));
	if (recog.is_verbose()) {
		cv::imwrite("debug_images/factory_text.png", factory_text);
	}
	std::vector<unsigned int> guids = recog.get_guid_from_name(factory_text, recog.get_dictionary().factories);
	if (guids.size() != 1)
		return std::make_pair(0, 0);

	if (recog.is_verbose()) {
		try {
			std::cout << recog.get_dictionary().factories.at(guids.front()) << ":\t";
		}
		catch (...) {}
		std::cout << std::endl;
	}

	std::vector<int> productivities;
	recog.iterate_rows(roi, 0.8f, [&](const cv::Mat& row)
		{
			int productivity = 0;
			for (const std::pair<float, float> cell : std::vector<std::pair<float, float>>({ {0.6f, 0.2f}, {0.8f, 0.2f} }))
			{
				cv::Mat productivity_text = recog.binarize(recog.get_cell(row, cell.first, cell.second));
				if (recog.is_verbose()) {
					cv::imwrite("debug_images/productivity_text.png", productivity_text);
				}
				int prod = recog.number_from_region(productivity_text);
				if (prod > 1000) // sometimes '%' is detected as '0/0' or '00'
					prod /= 100;

				if (prod >= 0)
					productivity += prod;
				else
					return;
			}

			if (recog.is_verbose()) {
				std::cout << std::endl;
			}

			productivities.push_back(productivity);

		});

	if (productivities.empty())
		return std::make_pair(0, 0);

	int sum = std::accumulate(productivities.begin(), productivities.end(), 0);

	if (recog.is_verbose()) {
		std::cout << " = " << sum << std::endl;
	}

	if (productivities.size() != buildings_count)
	{
		std::sort(productivities.begin(), productivities.end());
		sum += (buildings_count - productivities.size()) * productivities[productivities.size() / 2];
	}

	return std::make_pair(guids.front(), sum / buildings_count);
}



std::map<unsigned int, statistics_screen::properties> statistics_screen::get_factory_properties()
{
	const cv::Mat& im = screenshot;

	std::map<unsigned int, properties> result;
	if (get_open_tab() != statistics_screen::tab::PRODUCTION)
		return result;

	cv::Mat roi = get_center_pane();

	if (roi.empty())
		return result;

	if (recog.is_verbose()) {
		cv::imwrite("debug_images/statistics_window_scroll_area.png", roi);
	}

	if (recog.is_verbose()) {
		std::cout << "Average productivities" << std::endl;
	}

	recog.iterate_rows(roi, 0.9f, [&](const cv::Mat& row)
		{
			properties props;
		
			if (recog.is_verbose()) {
				cv::imwrite("debug_images/row.png", row);
			}

			cv::Mat product_icon = recog.get_square_region(row, statistics_screen_params::position_factory_icon);
			if (recog.is_verbose()) {
				cv::imwrite("debug_images/factory_icon.png", product_icon);
			}
			cv::Scalar background_color = statistics_screen::is_selected(product_icon.at<cv::Vec4b>(0, 0)) ? statistics_screen_params::background_blue_dark : statistics_screen_params::background_brown_light;

			std::vector<unsigned int> p_guids = recog.get_guid_from_icon(product_icon, recog.product_icons, background_color);
			if (p_guids.empty())
				return;

			if (recog.is_verbose()) {
				try {
					std::cout << recog.get_dictionary().products.at(p_guids.front()) << ":\t";
				}
				catch (...) {}
			}

			bool selected = is_selected(row.at<cv::Vec4b>(0.1f * row.rows, 0.5f * row.cols));
			cv::Mat productivity_text = recog.binarize(recog.get_cell(row, 0.7f, 0.1f, 0.4f), selected);
			if (recog.is_verbose()) {
				cv::imwrite("debug_images/productivity_text.png", productivity_text);
			}
			int prod = recog.number_from_region(productivity_text);

			if (prod > 500 && prod % 100 == 0)
				prod /= 100;

			if (prod >= 0)
				props.emplace(KEY_PRODUCTIVITY, prod);
		
			cv::Mat text_img = recog.binarize(recog.get_pane(statistics_screen_params::position_factory_output, row), true, true, 200);
			if (recog.is_verbose()) {
				cv::imwrite("debug_images/factory_output_text.png", text_img);
			}

			auto pair = recog.read_number_slash_number(text_img);

			if (pair.first >= 0)
				props.emplace(KEY_AMOUNT, pair.first);

			if (pair.second >= 0 && pair.second >= pair.first)
				props.emplace(KEY_LIMIT, pair.second);
		
			if (prod >= 0)
			{
				for (unsigned int p_guid : p_guids)
					for (unsigned int f_guid : recog.product_to_factories[p_guid])
						result.emplace(f_guid, props);
			}


			if (recog.is_verbose()) {
				std::cout << std::endl;
			}

		});


	return result;
}

std::map<unsigned int, int> statistics_screen::get_assets_existing_buildings_from_finance_screen()
{
	std::map<unsigned int, int> result;

	if (get_open_tab() != statistics_screen::tab::FINANCE)
		return result;

	cv::Mat roi = get_center_pane();

	if (roi.empty())
		return result;

	if (recog.is_verbose()) {
		cv::imwrite("debug_images/statistics_window_scroll_area.png", roi);
	}

	std::map<unsigned int, std::string> category_dict = recog.make_dictionary({ phrase::RESIDENTS, phrase::PRODUCTION });

	cv::Mat text = recog.binarize(get_right_header());

	std::vector<unsigned int> selection = recog.get_guid_from_name(text, category_dict);



	if (recog.is_verbose()) {
		try {
			std::cout << recog.get_dictionary().ui_texts.at(center_pane_selection) << std::endl;
		}
		catch (...) {}
	}

	if (selection.size() != 1)
		return result; // nothing selected that we want to evaluate

	center_pane_selection = selection.front();

	const std::map<unsigned int, std::string>* dictionary = nullptr;
	switch ((phrase)center_pane_selection)
	{
	case phrase::RESIDENTS:
		dictionary = &recog.get_dictionary().population_levels;
		break;
	case phrase::PRODUCTION:
		dictionary = &recog.get_dictionary().factories;
		break;
	}

	roi = get_right_pane();
	if (recog.is_verbose()) {
		cv::imwrite("debug_images/statistics_window_table_area.png", roi);
	}
	std::vector<unsigned int> prev_guids;
	int prev_count = 0;

	recog.iterate_rows(roi, 0.75f, [&](const cv::Mat& row)
		{
			if (recog.is_verbose()) {
				cv::imwrite("debug_images/row.png", row);
				cv::imwrite("debug_images/selection_test.png", row(cv::Rect((int)(0.037f * row.cols), (int)(0.5f * row.rows), 10, 10)));
			}
			bool is_summary_entry = image_recognition::closer_to(row.at<cv::Vec4b>(0.5f * row.rows, 0.037f * row.cols), statistics_screen_params::expansion_arrow, statistics_screen_params::background_brown_light);

			if (is_summary_entry)
			{
				prev_guids.clear();

				cv::Mat text = recog.binarize(recog.get_cell(row, 0.15f, 0.5f));

				std::vector<unsigned int> guids = recog.get_guid_from_name(text, *dictionary);


				if (recog.is_verbose()) {
					try {
						for (unsigned int guid : guids)
							std::cout << dictionary->at(guid) << ", ";
						std::cout << "\t";
					}
					catch (...) {}
				}

				cv::Mat count_text = recog.binarize(recog.get_cell(row, 0.15f, 0.5f));

				if (recog.is_verbose()) {
					cv::imwrite("debug_images/count_text.png", count_text);
				}

				std::vector<std::pair<std::string, cv::Rect>> words = recog.detect_words(count_text, tesseract::PSM_SINGLE_LINE);
				std::string number_string;
				bool found_opening_bracket = false;
				for (const auto& word : words)
				{
					if (found_opening_bracket)
						number_string += word.first;
					else
					{
						std::vector<std::string> split_string;
						boost::split(split_string, word.first, [](char c) {return c == '('; });
						if (split_string.size() > 1)
						{
							found_opening_bracket = true;
							number_string += split_string.back();
						}
					}
				}

				number_string = std::regex_replace(number_string, std::regex("\\D"), "");
				if (recog.is_verbose()) {
					std::cout << number_string;
				}
				int count = std::numeric_limits<int>::lowest();
				try { count = std::stoi(number_string); }
				catch (...) {
					if (recog.is_verbose()) {
						std::cout << " (could not recognize number)";
					}
				}

				if (count >= 0)
				{
					if (guids.size() != 1 && get_selected_session() && !is_all_islands_selected())
						recog.filter_factories(guids, get_selected_session());

					if (guids.size() != 1) {
						cv::Mat product_icon = recog.get_square_region(row, statistics_screen_params::position_small_factory_icon);
						if (recog.is_verbose()) {
							cv::imwrite("debug_images/factory_icon.png", product_icon);
						}
						cv::Scalar background_color = statistics_screen_params::background_brown_light;
						std::map<unsigned int, cv::Mat> icon_candidates;
						for (unsigned int guid : guids)
						{
							auto iter = recog.factory_icons.find(guid);
							if (iter != recog.factory_icons.end())
								icon_candidates.insert(*iter);
						}

						guids = recog.get_guid_from_icon(product_icon, icon_candidates, background_color);
					}

					if (guids.size() == 1)
						result.emplace(guids.front(), count);
					else
					{
						prev_guids = guids;
						prev_count = count;
					}
				}

			}
			else if (!prev_guids.empty()) // no summary entry, test whether upper row is expanded
			{
				cv::Mat session_icon = recog.get_cell(row, 0.08f, 0.08f);
				if (recog.is_verbose()) {
					cv::imwrite("debug_images/session_icon.png", session_icon);
				}

				unsigned int session_guid = recog.get_session_guid(session_icon);
				if (!session_guid)
				{
					// prev_guids.clear();
					return;
				}

				recog.filter_factories(prev_guids, session_guid);

				if (prev_guids.size() == 1)
					result.emplace(prev_guids.front(), prev_count);

				prev_guids.clear();

			}
			if (recog.is_verbose()) {
				std::cout << std::endl;
			}
		});

	return result;
}

std::map<unsigned int, statistics_screen::properties> statistics_screen::get_population_properties() const
{
	const cv::Mat& im = screenshot;

	std::map<unsigned int, properties> result;

	if (get_open_tab() != statistics_screen::tab::POPULATION)
		return result;

	cv::Mat roi = get_center_pane();

	if (roi.empty())
		return result;

	if (recog.is_verbose()) {
		cv::imwrite("debug_images/statistics_window_scroll_area.png", roi);
	}

	recog.iterate_rows(roi, 0.75f, [&](const cv::Mat& row)
		{
			properties props;

			cv::Mat population_name = recog.binarize(recog.get_cell(row, 0.076f, 0.2f));
			if (recog.is_verbose()) {
				cv::imwrite("debug_images/population_name.png", population_name);
			}
			std::vector<unsigned int> guids = recog.get_guid_from_name(population_name, recog.get_dictionary().population_levels);
			if (guids.size() != 1)
				return;

			// read amount and limit
			cv::Mat text_img = recog.binarize(recog.get_cell(row, 0.5f, 0.27f, 0.4f));
			if (recog.is_verbose()) {
				cv::imwrite("debug_images/pop_amount_text.png", text_img);
			}


			if (recog.is_verbose()) {
				try {
					std::cout << recog.get_dictionary().population_levels.at(guids.front()) << "\t";
				}
				catch (...) {}
			}

			auto pair = recog.read_number_slash_number(text_img);


			if (pair.first >= 0)
				props.emplace(KEY_AMOUNT, pair.first);

			if (pair.second >= 0 && pair.second >= pair.first)
				props.emplace(KEY_LIMIT, pair.second);

			// read existing buildings
			text_img = recog.binarize(recog.get_cell(row, 0.3f, 0.15f, 0.4f));
			if (recog.is_verbose()) {
				cv::imwrite("debug_images/pop_houses_text.png", text_img);
			}
			int houses = recog.number_from_region(text_img);

			if (houses >= 0)
				props.emplace(KEY_EXISTING_BUILDINGS, houses);

			if (recog.is_verbose()) {
				std::cout << std::endl;
			}

			if (!props.empty())
				result.emplace(guids.front(), props);
		});

	if (result.size() < 6)
		for (const auto& entry : recog.get_dictionary().population_levels)
		{
			if (result.find(entry.first) == result.end())
				result.emplace(entry.first,
					std::map<std::string, int>({
					{KEY_AMOUNT, 0},
					{KEY_EXISTING_BUILDINGS, 0},
					{KEY_LIMIT, 0}
						}));
		}
	
	return result;
}

std::map<std::string, unsigned int> statistics_screen::get_islands() const
{
	return island_to_session;
}

std::map<std::string, unsigned int> statistics_screen::get_current_islands() const
{
	return current_island_to_session;
}

std::string statistics_screen::get_selected_island()
{
	if (!selected_island.empty() || multiple_islands_selected)
		return selected_island;

	if (is_all_islands_selected())
	{
		if (recog.is_verbose()) {
			std::cout << recog.ALL_ISLANDS << std::endl;
		}
		selected_session = recog.SESSION_META;
		selected_island = recog.ALL_ISLANDS;
		return selected_island;
	}

	std::string result;

	cv::Mat roi = recog.binarize(get_center_header());

	if (roi.empty())
		return result;

	if (recog.is_verbose()) {
		cv::imwrite("debug_images/header.png", roi);
	}

	auto words_and_boxes = recog.detect_words(roi, tesseract::PSM_SINGLE_LINE);


	// check whether mutliple islands are selected
	std::string joined_string = recog.join(words_and_boxes);
	std::vector<std::string> split_string;
	boost::split(split_string, joined_string, [](char c) {return c == '/' || c == '[' || c == '(' || c == '{'; });
	if (!recog.get_guid_from_name(split_string.front(), recog.make_dictionary({ phrase::MULTIPLE_ISLANDS })).empty())
	{
		if (recog.is_verbose()) {
			std::cout << recog.get_dictionary().ui_texts.at((unsigned int)phrase::MULTIPLE_ISLANDS) << std::endl;
		}

		multiple_islands_selected = true;
		return selected_island;
	}

	int max_dist = 0;
	int last_index = 0;

	// detect split between island and session
	for (int i = 0; i < words_and_boxes.size() - 1; i++) {
		int dist = words_and_boxes[i + 1].second.x -
			(words_and_boxes[i].second.x + words_and_boxes[i].second.width);
		if (dist > max_dist)
		{
			max_dist = dist;
			last_index = i;
		}
	}

	std::string island;
	std::string session;
	for (int i = 0; i < words_and_boxes.size(); i++)
	{
		if (i <= last_index)
			island += words_and_boxes[i].first + " ";
		else
			session += words_and_boxes[i].first;
	}
	island.pop_back();

	selected_island = island;


	auto session_result = recog.get_guid_from_name(session, recog.make_dictionary({
		phrase::THE_ARCTIC,
		phrase::THE_OLD_WORLD,
		phrase::THE_NEW_WORLD,
		phrase::CAPE_TRELAWNEY,
		phrase::ENBESA
		}));

	if (session_result.size())
	{
		selected_session = session_result[0];
		island_to_session.emplace(selected_island, selected_session);
		current_island_to_session.emplace(selected_island, selected_session);
	}

	if (recog.is_verbose()) {
		std::cout << result << std::endl;
	}
	return selected_island;
}

unsigned int statistics_screen::get_selected_session()
{
	get_selected_island(); // update island information
	return selected_session;
}




std::map<unsigned int, int> statistics_screen::get_population_workforce() const
{
	const cv::Mat& im = screenshot;

	std::map<unsigned int, int> result;

	if (get_open_tab() != statistics_screen::tab::POPULATION)
		return result;

	cv::Mat roi = get_center_pane();

	if (roi.empty())
		return result;

	if (recog.is_verbose()) {
		cv::imwrite("debug_images/statistics_window_scroll_area.png", roi);
	}

	recog.iterate_rows(roi, 0.75f, [&](const cv::Mat& row)
		{
			cv::Mat population_name = recog.binarize(recog.get_cell(row, 0.076f, 0.2f));
			if (recog.is_verbose()) {
				cv::imwrite("debug_images/population_name.png", population_name);
			}
			std::vector<unsigned int> guids = recog.get_guid_from_name(population_name, recog.get_dictionary().population_levels);
			if (guids.size() != 1)
				return;

			cv::Mat text_img = recog.binarize(recog.get_cell(row, 0.8f, 0.1f));
			if (recog.is_verbose()) {
				cv::imwrite("debug_images/pop_houses_text.png", text_img);
			}
			int workforce = recog.number_from_region(text_img);

			if (workforce >= 0)
				result.emplace(guids.front(), workforce);

		});

	for (const auto& entry : recog.get_dictionary().population_levels)
	{
		if (result.find(entry.first) == result.end())
			result[entry.first] = 0;
	}
	return result;
}

}
