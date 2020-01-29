#include "reader_util.hpp"



#define NOMINMAX
#include <windows.h>

#include <algorithm>
#include <regex>
#include <list>
#include <filesystem>
#include <numeric>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/cxx17/transform_reduce.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <opencv2/opencv.hpp>


#include <tesseract/genericvector.h>

// #define SHOW_CV_DEBUG_IMAGE_VIEW
// #define CONSOLE_DEBUG_OUTPUT


////////////////////////////////////////
//
// Class: statistics_screen
//
////////////////////////////////////////


statistics_screen::statistics_screen(image_recognition& recog)
	:
	recog(recog),
	open_tab(tab::NONE)
{
}

void statistics_screen::update(const cv::Mat& screenshot)
{
	screenshot.copyTo(this->screenshot);

	cv::Mat& src = this->screenshot;
	// handle 21:9 widescreen where statistics screen is shown 16:9 with black bars
	if (src.rows && (src.cols / (float) src.rows) >= 2.33)
	{
		int width = src.rows * 16 / 9;
		int crop = (src.cols - width) / 2;
		cv::Rect roi(crop, 0, width, src.rows);
		src = src(roi);
	}

	// test if open

	cv::Mat statistics_text_img = recog.binarize(get_pane(pane_title), true);
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/statistics_text.png", statistics_text_img);
#endif
	if (recog.get_guid_from_name(statistics_text_img, recog.make_dictionary({ phrase::STATISTICS })).empty())
	{
		open_tab = tab::NONE;
#ifdef CONSOLE_DEBUG_OUTPUT
		std::cout << std::endl;
#endif
		return;
	}

#ifdef CONSOLE_DEBUG_OUTPUT
	std::cout << std::endl;
#endif

	open_tab = compute_open_tab();
	if (open_tab == tab::NONE)
		return;
	bool update = true;
	if (prev_islands.size().area() == get_pane(pane_islands).size().area())
	{
		cv::Mat diff;
		cv::absdiff(get_pane(pane_islands), prev_islands, diff);
		std::cout << ((float)cv::sum(diff).ddot(cv::Scalar::ones())) << std::endl;
		float match = ((float) cv::sum(diff).ddot(cv::Scalar::ones())) / prev_islands.rows / prev_islands.cols;
		update = match > 30;
	}
	if (update)
	{ // island list changed
		get_pane(pane_islands).copyTo(prev_islands);
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
	cv::Mat tabs = get_pane(pane_tabs);
	int tabs_count = (int)tab::POPULATION;
	int tab_width = tabs.cols / tabs_count;
	int v_center = tabs.rows / 2;
	for (int i = 1; i <= (int) tabs_count; i++)
	{
		cv::Vec4b pixel = tabs.at<cv::Vec4b>(v_center, (int)(i * tab_width - 0.2f * tab_width));
		if (is_tab_selected(pixel))
		{
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
			cv::imwrite("debug_images/tab.png", tabs(cv::Rect(i * tab_width - 0.2f * tab_width, v_center, 10,10)));
#endif
#ifdef CONSOLE_DEBUG_OUTPUT
			std::cout << "Open tab:\t" << i << std::endl;
#endif
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
		phrase::CAPE_TRELAWNEY });

	recog.iterate_rows(prev_islands, [&](const cv::Mat& row) {
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
		cv::imwrite("debug_images/row.png", row);
#endif

		cv::Mat subheading = recog.binarize(get_cell(row, 0.01f, 0.6f, 0.f), true);

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
		cv::imwrite("debug_images/subheading.png", subheading);
#endif

		std::vector<unsigned int> ids = recog.get_guid_from_name(subheading, phrases);
		if (ids.size() == 1)
		{
			session_guid = ids.front();
			return;
		}

		

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
		cv::imwrite("debug_images/selection_test.png", row(cv::Rect((int) (0.8f * row.cols), (int) (0.5f * row.rows),  10, 10)));
#endif

		bool selected = is_selected(row.at<cv::Vec4b>((int) (0.5f * row.rows), (int) (0.8f * row.cols)));
		cv::Mat island_name_image = recog.binarize(get_cell(row, 0.15f, 0.65f), selected);

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
		cv::imwrite("debug_images/island_name.png", island_name_image);
#endif

		std::string island_name = recog.join(recog.detect_words(island_name_image), true);
		if (island_name.empty())
			return;

		if (get_island_from_list(island_name).second)
			return;

		cv::Mat session_icon = get_cell(row, 0.025f, 0.14f);
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
		cv::imwrite("debug_images/session_icon.png", session_icon);
#endif
		session_guid = recog.get_session_guid(session_icon);

		if (session_guid) 
			island_to_session.emplace(island_name, session_guid);

#ifdef CONSOLE_DEBUG_OUTPUT
		std::cout << "Island added:\t" << island_name;
		try {
			std::cout << " (" << recog.get_dictionary().ui_texts.at(session_guid) << ")";
		}
		catch (const std::exception& e) {}
		std::cout << std::endl;
#endif

	});
}

bool statistics_screen::is_all_islands_selected() const
{
	if (!screenshot.size || !is_open())
		return false;

	cv::Mat button = get_pane(pane_all_islands);
	if (button.empty())
		return false;

	return is_selected(button.at<cv::Vec4b>(0.1f * button.rows, 0.5f * button.cols));
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
		return get_pane(pane_finance_center);
	case tab::PRODUCTION:
		return get_pane(pane_production_center);
	case tab::POPULATION:
		return get_pane(pane_population_center);
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
		return get_pane(pane_islands);
	}
}

cv::Mat statistics_screen::get_right_pane() const
{
	switch (get_open_tab())
	{
	case tab::FINANCE:
		return get_pane(pane_finance_right);
	case tab::PRODUCTION:
		return get_pane(pane_production_right);
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
		return get_pane(pane_header_center);
	}
}

cv::Mat statistics_screen::get_right_header() const
{
	switch (get_open_tab())
	{
	case tab::NONE:
		return cv::Mat();
	default:
		return get_pane(pane_header_right);
	}
}

bool statistics_screen::is_selected(const cv::Vec4b& point)
{
	return closer_to(point, background_blue_dark, background_brown_light);
}

bool statistics_screen::is_tab_selected(const cv::Vec4b& point)
{
	return closer_to(point, cv::Scalar(234, 205, 149, 255), cv::Scalar(205, 169, 119, 255));
}

bool statistics_screen::closer_to(const cv::Scalar& color, const cv::Scalar& ref, const cv::Scalar& other)
{
	return (color - ref).dot(color - ref) < (color - other).dot(color - other);
}

cv::Mat statistics_screen::get_square_region(const cv::Mat& img, const cv::Rect2f& rect)
{
	if (!img.size)
		return cv::Mat();

	int dim = std::lround(std::max(rect.width * img.cols, rect.height * img.rows));
	cv::Rect scaled(rect.x * img.cols, 
		rect.y * img.rows, 
		std::min(dim, (int) (img.cols - rect.x * img.cols)), 
		std::min(dim, (int) (img.rows - rect.y * img.rows)));
	return img(scaled);
}

cv::Mat statistics_screen::get_cell(const cv::Mat& img, float crop_left, float width, float crop_vertical)
{
	if (!img.size)
		return cv::Mat();

	cv::Rect scaled(crop_left * img.cols, 0.5f * crop_vertical * img.rows, width * img.cols, (1 - crop_vertical) * img.rows);
	return img(scaled);
}

cv::Mat statistics_screen::get_pane(const cv::Rect2f& rect) const
{
	if (!screenshot.size)
		return cv::Mat();

	cv::Point2f factor(screenshot.cols - 1, screenshot.rows - 1);
	cv::Rect scaled(cv::Point(rect.tl().x * factor.x, rect.tl().y * factor.y), 
		cv::Point(rect.br().x * factor.x, rect.br().y * factor.y));
	return screenshot(scaled);
}

const cv::Scalar statistics_screen::background_blue_dark = cv::Scalar(103, 87, 79, 255);
const cv::Scalar statistics_screen::background_brown_light = cv::Scalar(124, 181, 213, 255);
const cv::Scalar statistics_screen::foreground_brown_light = cv::Scalar(96, 195, 255, 255);
const cv::Scalar statistics_screen::foreground_brown_dark = cv::Scalar(19, 53, 81, 255);
const cv::Scalar statistics_screen::expansion_arrow = cv::Scalar(78, 98, 115, 255);

const cv::Rect2f statistics_screen::pane_tabs = cv::Rect2f(cv::Point2f(0.2883f, 0.147f), cv::Point2f(0.7118f, 0.1839f));
const cv::Rect2f statistics_screen::pane_title = cv::Rect2f(cv::Point2f(0.3839f, 0), cv::Point2f(0.6176f, 0.0722f));
const cv::Rect2f statistics_screen::pane_all_islands = cv::Rect2f(cv::Point2f(0.0234f, 0.2658f), cv::Point2f(0.19f, 0.315f));
const cv::Rect2f statistics_screen::pane_islands = cv::Rect2f(cv::Point2f(0.0215f, 0.3326f), cv::Point2f(0.1917f, 0.9586f));
const cv::Rect2f statistics_screen::pane_finance_center = cv::Rect2f(cv::Point2f(0.2471f, 0.3084f), cv::Point2f(0.5805f, 0.9576f));
const cv::Rect2f statistics_screen::pane_finance_right = cv::Rect2f(cv::Point2f(0.6261f, 0.3603f), cv::Point2f(0.9635f, 0.9587f));
const cv::Rect2f statistics_screen::pane_production_center = cv::Rect2f(cv::Point2f(0.246f, 0.3638f), cv::Point2f(0.5811f, 0.9567f));
const cv::Rect2f statistics_screen::pane_production_right = cv::Rect2f(cv::Point2f(0.629f, 0.3613f), cv::Point2f(0.9619f, 0.936f));
const cv::Rect2f statistics_screen::pane_population_center = cv::Rect2f(cv::Point2f(0.247f, 0.3571f), cv::Point2f(0.5802f, 0.7006f));
const cv::Rect2f statistics_screen::pane_header_center = cv::Rect2f(cv::Point2f(0.2453f, 0.2238f), cv::Point2f(0.4786f, 0.2581f));
const cv::Rect2f statistics_screen::pane_header_right = cv::Rect2f(cv::Point2f(0.6276f, 0.2238f), cv::Point2f(0.7946f, 0.2581f));

const cv::Rect2f statistics_screen::position_factory_icon = cv::Rect2f(0.0219f, 0.135f, 0.0838f, 0.7448f);
const cv::Rect2f statistics_screen::position_small_factory_icon = cv::Rect2f(0.013f, 0.05f, 0.09f, 0.7f);
const cv::Rect2f statistics_screen::position_population_icon = cv::Rect2f(0.f, 0.05f, 0.f, 0.9f);




struct comparePoints {
	bool operator()(const cv::Point& a, const cv::Point& b) const {
		if (a.x != b.x)
			return a.x < b.x;
		return a.y < b.y;
	}
};



////////////////////////////////////////
//
// Class: image_recognition
//
////////////////////////////////////////


image_recognition::image_recognition()
	:
	stats_screen(*this),
	language("english"),
	selected_island(),
	center_pane_selection(0),
	population_icon_position(cv::Rect(-1,-1, 0, 0))
{
	boost::property_tree::ptree pt;
	boost::property_tree::read_json("texts/params_2020-01-06_iconnames.json", pt);

	for(const auto& language : pt.get_child("languages"))
	{
		std::string key(language.second.get_value<std::string>());
		keyword_dictionary value;
		dictionaries.emplace(key, value);
	}

	auto load_and_save_icon = [](unsigned int guid, 
		const boost::property_tree::ptree& asset, 
		std::map<unsigned int, cv::Mat>& container)
	{
		if (asset.get_child_optional("icon").has_value())
		{
			try
			{
				cv::Mat icon = load_image("icons/" + asset.get_child("icon").get_value<std::string>());
				container.emplace(guid, icon);

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
				cv::imwrite("debug_images/icon_template.png", icon);
#endif
			}
			catch (const std::invalid_argument& e)
			{
				std::cout << e.what() << std::endl;
			}
		}
	};

	// load sessions and regions
#ifdef CONSOLE_DEBUG_OUTPUT
	std::cout << "Load sessions and regions." << std::endl;
#endif
	session_icons.emplace(180023, binarize_icon(load_image("icons/icon_session_moderate_white.png")));
	session_to_region.emplace(180023, 5000000);
	session_icons.emplace(180045, binarize_icon(load_image("icons/icon_session_passage_white.png")));
	session_to_region.emplace(180045, 160001);
	session_icons.emplace(180025, binarize_icon(load_image("icons/icon_session_southamerica_white.png")));
	session_to_region.emplace(180025, 5000001);
	session_icons.emplace(110934, binarize_icon(load_image("icons/icon_session_sunken_treasure_white.png")));
	session_to_region.emplace(110934, 5000000);

	// load factories
	auto process_factories = [&](const boost::property_tree::ptree& root) {
		for (const auto& factory : root)
		{
			unsigned int guid = factory.second.get_child("guid").get_value<unsigned int>();
			if (factory.second.get_child_optional("region").has_value())
			{
				factory_to_region.emplace(guid, factory.second.get_child("region").get_value<unsigned int>());
			}

			load_and_save_icon(guid, factory.second, factory_icons);

			for (const auto& language : factory.second.get_child("locaText"))
			{

				dictionaries.at(language.first).factories.emplace(guid, language.second.get_value<std::string>());
			}
		}
	};

#ifdef CONSOLE_DEBUG_OUTPUT
	std::cout << "Load factories." << std::endl;
#endif
	process_factories(pt.get_child("factories"));
	process_factories(pt.get_child("powerPlants"));

	// load products
	for (const auto& product : pt.get_child("products"))
	{
		unsigned int guid = product.second.get_child("guid").get_value<unsigned int>();
		if (!product.second.get_child_optional("producers").has_value())
			continue;

		// store factories and regions
		std::vector<unsigned int> factories;
		for (const auto& factory_entry : product.second.get_child("producers"))
		{
			unsigned int factory_id = factory_entry.second.get_value<unsigned int>();
			factories.push_back(factory_id);
		}
		product_to_factories.emplace(guid, std::move(factories));

		for (const auto& language : product.second.get_child("locaText"))
		{
			dictionaries.at(language.first).products.emplace(guid, language.second.get_value<std::string>());
		}

		load_and_save_icon(guid, product.second, product_icons);
	}

#ifdef CONSOLE_DEBUG_OUTPUT
	std::cout << "Load population levels." << std::endl;
#endif
	// load population levels
	for (const auto& level : pt.get_child("populationLevels"))
	{
		unsigned int guid = level.second.get_child("guid").get_value<unsigned int>();
		load_and_save_icon(guid, level.second, population_icons);
		for (const auto& language : level.second.get_child("locaText"))
		{

			dictionaries.at(language.first).population_levels.emplace(guid, language.second.get_value<std::string>());
		}
	}

#ifdef CONSOLE_DEBUG_OUTPUT
	std::cout << "Load texts." << std::endl;
#endif
	pt.clear();
	if (boost::filesystem::exists("texts/ui_texts.json"))
	{
		boost::property_tree::read_json("texts/ui_texts.json", pt);
		for (const auto& text_node : pt)
		{
			std::string language = text_node.first;
			for (const auto& entry : text_node.second)
			{
				unsigned int guid = std::atoi(entry.first.c_str());
				dictionaries.at(language).ui_texts.emplace(guid, entry.second.get_value<std::string>());
			}
		}
	} 
	else
	{
		std::set<phrase> phrases({
			phrase::ALL_ISLANDS,
			phrase::PRODUCTION,
			phrase::PRODUCTIVITY,
			phrase::STATISTICS,
			phrase::THE_NEW_WORLD,
			phrase::THE_OLD_WORLD,
			phrase::CAPE_TRELAWNEY,
			phrase::THE_ARCTIC,
			phrase::RESIDENTS,
			phrase::BREAKDOWN
			});

		boost::property_tree::ptree output_json;

		for (auto& entry : dictionaries)
		{
			const std::string& language = entry.first;

			pt.clear();
			boost::property_tree::read_xml("texts/texts_" + language + ".xml", pt);

			boost::property_tree::ptree texts;

			for (const auto& text_node : pt.get_child("TextExport").get_child("Texts"))
			{
				unsigned int guid = text_node.second.get_child("GUID").get_value<unsigned int>();
				if (phrases.find((phrase)guid) != phrases.end())
				{
					std::string loca_text = text_node.second.get_child("Text").get_value<std::string>();
					entry.second.ui_texts[guid] = loca_text;
					texts.put(std::to_string(guid), loca_text);
				}
			}

			output_json.add_child(language, texts);
		}

		boost::property_tree::write_json("texts/ui_texts.json", output_json);

	}
}

std::string image_recognition::to_string(const std::wstring& str)
{
	return std::string(str.begin(), str.end());
}

std::wstring image_recognition::to_wstring(const std::string& str)
{
	return std::wstring(str.begin(), str.end());
}

std::list<cv::Point> image_recognition::find_rgb_region(cv::InputArray in, const cv::Point& seed, float threshold)
{
	cv::Mat input = in.getMat();
	std::list<cv::Point> ret;

	if (seed.x >= input.cols || seed.y >= input.rows)
		return ret;

	const cv::Rect img_rect = cv::Rect({ 0,0 }, in.size());

	std::set < cv::Point, comparePoints> open;
	open.insert(seed);
	const auto seed_color = input.at<cv::Vec4b>(seed);

	std::set<cv::Point, comparePoints> closed;



	while (!open.empty()) {
		const cv::Point current_point = *open.begin();
		open.erase(open.begin());


		const auto cc = input.at<cv::Vec4b>(current_point);
		const int color_diff = (cc.val[0] - int(seed_color.val[0])) * (cc.val[0] - int(seed_color.val[0]))
			+ (cc.val[1] - int(seed_color.val[1])) * (cc.val[1] - int(seed_color.val[1]))
			+ (cc.val[2] - int(seed_color.val[2])) * (cc.val[2] - int(seed_color.val[2]))
			+ (cc.val[3] - int(seed_color.val[3])) * (cc.val[3] - int(seed_color.val[3]));
		if (color_diff
			<= threshold)
		{
			ret.push_back(current_point);
			cv::Point new_candidates[4] = {
				current_point + cv::Point(1,0),
				current_point + cv::Point(0,1),
				current_point + cv::Point(-1,0),
				current_point + cv::Point(0,-1)
			};
			for (int i = 0; i < 4; i++) {
				if (new_candidates[i].inside(img_rect)
					&& closed.find(new_candidates[i]) == closed.end())
				{
					open.insert(new_candidates[i]);
				}
			}
		}
		closed.insert(current_point);
	}
	return ret;
}

cv::Mat image_recognition::blend_icon(cv::InputArray icon, cv::Scalar background_color)
{
	cv::Mat background_img = cv::Mat(icon.rows(), icon.cols(), CV_8UC4);
	cv::Mat zeros = cv::Mat(icon.rows(), icon.cols(), CV_8UC1,cv::Scalar(0));
	background_img = background_color;

	std::vector<cv::Mat> icon_channels;
	cv::split(icon, icon_channels);
	cv::Mat alpha;
	cv::merge(std::vector<cv::Mat>({ icon_channels[3],icon_channels[3],icon_channels[3],zeros }), alpha);

	return background_img.mul(cv::Scalar(255, 255, 255, 255) - alpha, 1./255) + icon.getMat().mul(alpha, 1./255);
}

cv::Mat image_recognition::dye_icon(cv::InputArray icon, cv::Scalar color)
{
	cv::Mat blue = cv::Mat(icon.rows(), icon.cols(), CV_8UC1, cv::Scalar(color[0]));
	cv::Mat green = cv::Mat(icon.rows(), icon.cols(), CV_8UC1, cv::Scalar(color[1]));
	cv::Mat red = cv::Mat(icon.rows(), icon.cols(), CV_8UC1, cv::Scalar(color[2]));

	std::vector<cv::Mat> icon_channels;
	cv::split(icon, icon_channels);
	cv::Mat result;
	cv::merge(std::vector<cv::Mat>({ blue, green, red, icon_channels[3] }), result);

	return result;
}

std::pair<cv::Rect, float> image_recognition::find_icon(cv::InputArray source, cv::InputArray icon, cv::Scalar background_color)
{
	float scaling = (source.cols() * 0.027885)/icon.cols();
	
	cv::Mat template_resized;
	cv::resize(blend_icon(icon, background_color), template_resized, cv::Size(scaling * icon.rows(), scaling * icon.cols()));



#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/icon_template.png", template_resized);
#endif
	return match_template(source, template_resized);
}


std::vector<unsigned int> image_recognition::get_guid_from_icon(cv::Mat icon,
	const std::map<unsigned int, cv::Mat>& dictionary) const
{
	if (icon.empty())
		return std::vector<unsigned int>();

	cv::Scalar background_color = statistics_screen::is_selected(icon.at<cv::Vec4b>(0, 0)) ? statistics_screen::background_blue_dark : statistics_screen::background_brown_light;
	cv::Mat background(icon.rows, icon.cols, CV_8UC4, background_color);
	
	cv::Mat diff;
	cv::absdiff(icon, background, diff);
	float best_match = cv::sum(diff).ddot(cv::Scalar::ones()) / icon.rows / icon.cols;
	std::vector<unsigned int> guids;


	for (auto& entry : dictionary)
	{
		

		cv::Mat template_resized;
		cv::resize(blend_icon(entry.second, background_color), template_resized, cv::Size(icon.cols, icon.rows));

		cv::Mat diff;
		cv::absdiff(icon, template_resized, diff);
		float match = cv::sum(diff).ddot(cv::Scalar::ones()) / icon.rows / icon.cols;
		if (match == best_match)
		{
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
			cv::imwrite("debug_images/icon_template.png", template_resized);
#endif
			guids.push_back(entry.first);
		}
		else if (match < best_match)
		{
			guids.clear();
			guids.push_back(entry.first);
			best_match = match;
		}
	}

	if (best_match > 150)
		return std::vector<unsigned int>();

#ifdef CONSOLE_DEBUG_OUTPUT
	for (unsigned int guid : guids)
		std::cout << guid << ", ";
	std::cout << "(" << best_match << ")\t";
#endif
	return guids;
}


unsigned int image_recognition::get_session_guid(cv::Mat icon) const
{
	if (icon.empty())
		return 0;

	float best_match = 0;
	unsigned int guid = 0;

	for (auto& entry : session_icons)
	{

		cv::Mat icon_processed = binarize_icon(icon, entry.second.size());
		int icon_white_count = cv::countNonZero(icon_processed);
		cv::bitwise_and(entry.second, icon_processed, icon_processed);

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
		cv::imwrite("debug_images/icon_intersect.png", icon_processed);
#endif
		
		float max_intersection = std::max(icon_white_count, cv::countNonZero(entry.second));
		float match = cv::countNonZero(icon_processed) / max_intersection;

#ifdef CONSOLE_DEBUG_OUTPUT
		std::cout << "\t(" << entry.first << ", " << match << ")";
#endif
		if (match > best_match)
		{
			guid = entry.first;
			best_match = match;
		}
	}

#ifdef CONSOLE_DEBUG_OUTPUT
	std::cout << std::endl;
#endif

	if (best_match < 0.7f)
		return 0;

	return guid;
}



std::vector<unsigned int> image_recognition::get_guid_from_name(const cv::Mat& text_img,
	const std::map<unsigned int, std::string>& dictionary) const
{
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/text_img.png", text_img);
#endif
	std::vector<std::pair<std::string, cv::Rect>> words = detect_words(text_img, tesseract::PageSegMode::PSM_SINGLE_LINE);
	std::string building_string;
	for (const auto& word : words)
	{
		//stop concatenation before opening bracket
		std::vector<std::string> split_string;
		boost::split(split_string, word.first, [](char c) {return c == '('; });
		if (split_string.size() > 1)
		{
			building_string += split_string.front();
			break;
		}
		else
		{
			building_string += word.first;
		}
	}

#ifdef CONSOLE_DEBUG_OUTPUT
	std::cout << building_string << "\t";
#endif

	std::vector<unsigned int> guids;
	float best_match = 0.f;
	for (const auto& entry : dictionary)
	{
		std::vector<std::string> split_string;
		boost::split(split_string, entry.second, [](char c) {return c == ' '; });
		std::string kw = boost::join(split_string, "");

		float match = lcs_length(kw, building_string) / (float) std::max(kw.size(), building_string.size());
		if (match > 0.66f)
		{
			if (match == best_match)
			{
				guids.push_back(entry.first);
			}
			else if (match > best_match)
			{
				guids.clear();
				guids.push_back(entry.first);
				best_match = match;
			}
		}
	}
	return guids;
}

void image_recognition::filter_factories(std::vector<unsigned int>& factories, unsigned int session) const
{
	if (session_to_region.find(session) == session_to_region.end())
		return;

	unsigned int region = session_to_region.at(session);
	for (auto iter = factories.begin(); iter != factories.end(); )
	{
		if (factory_to_region.at(*iter) == region)
			++iter;
		else
			iter = factories.erase(iter);
	}
}

double image_recognition::compare_hu_moments(const std::vector<double>& ma, const std::vector<double>& mb)
{
	// execute return cv::matchShapes(ma, mb, cv::CONTOURS_MATCH_I2, 0.);
	// with pre-computed hu moments

	int i, sma, smb;
	double eps = 1.e-5;
	double result = 0;
	bool anyA = false, anyB = false;


	for (i = 0; i < 7; i++)
	{
		double ama = fabs(ma[i]);
		double amb = fabs(mb[i]);

		if (ama > 0)
			anyA = true;
		if (amb > 0)
			anyB = true;

		if (ma[i] > 0)
			sma = 1;
		else if (ma[i] < 0)
			sma = -1;
		else
			sma = 0;
		if (mb[i] > 0)
			smb = 1;
		else if (mb[i] < 0)
			smb = -1;
		else
			smb = 0;

		if (ama > eps && amb > eps)
		{
			ama = sma * log10(ama);
			amb = smb * log10(amb);
			result += fabs(-ama + amb);
		}
	}

	if (anyA != anyB)
		result = DBL_MAX;

	return result;
}



std::pair<cv::Rect, float> image_recognition::match_template(cv::InputArray source, cv::InputArray template_img)
{
	cv::Mat src_hs = convert_color_space_for_template_matching(source);
	cv::Mat tmpl_hs = convert_color_space_for_template_matching(template_img);

	cv::Mat result;
	cv::matchTemplate(src_hs, tmpl_hs, result, cv::TM_SQDIFF);
	result /= tmpl_hs.size().area();
	cv::Point min_loc, max_loc;
	double min, max;
	cv::minMaxLoc(result, &min, &max, &min_loc, &max_loc);

	cv::Point template_position = min_loc;

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/match_image.bmp", (result) * 256 / max);
#endif //SHOW_CV_DEBUG_IMAGE_VIEW

	return { cv::Rect(template_position, tmpl_hs.size()), min };
}



std::map<unsigned int, int> image_recognition::get_population_amount()
{
	if (stats_screen.is_open())
		return get_population_amount_from_statistic_screen();
	else
		return get_population_amount_from_hud();
}

cv::Rect image_recognition::find_population_icon()
{
	if (population_icon_position.tl().x >= 0)
		return population_icon_position;

	const auto fit_criterion = [](float e) {return e < 20000.f; };

	struct template_images {
		std::string resolution_id = "uninitialized";
		cv::Mat island_pop_symbol;
	};

	static template_images templates;
	std::string resolution_id = std::to_string(screenshot.cols) + "x" + std::to_string(screenshot.rows);
	if (templates.resolution_id != resolution_id)
	{
		try {
#ifdef CONSOLE_DEBUG_OUTPUT
			std::cout << "detected resolution: " << resolution_id
				<< std::endl;
#endif
			templates.island_pop_symbol = load_image("image_recon/" + resolution_id + "/population_symbol_with_bar.bmp");
			templates.resolution_id = resolution_id;
		}
		catch (const std::invalid_argument& e) {
			std::cout << e.what() << ". Make sure the Anno 1800 is focused and has proper resolution!" << std::endl;
			population_icon_position = cv::Rect(0, 0, 0, 0);
			return population_icon_position;
		}
	}


	cv::Mat im_copy = screenshot(cv::Rect(cv::Point(0, 0), cv::Size(screenshot.cols, screenshot.rows / 2)));

	const auto pop_symbol_match_result = match_template(im_copy, templates.island_pop_symbol);

	if (!fit_criterion(pop_symbol_match_result.second)) {
#ifdef CONSOLE_DEBUG_OUTPUT
		std::cout << "can't find population" << std::endl;
#endif
		population_icon_position = cv::Rect(0,0,0,0);
	}
	population_icon_position = pop_symbol_match_result.first;

	return population_icon_position;
}


std::map<unsigned int, int> image_recognition::get_population_amount_from_hud()
{
	cv::Rect pop_icon_position = find_population_icon();

	if(pop_icon_position.tl().x <= 0)
		return std::map<unsigned int, int>();

	auto region = find_rgb_region(screenshot, pop_icon_position.br(), 0);
	cv::Rect aa_bb = get_aa_bb(region);
	if (aa_bb.area() <= 0)
		return std::map<unsigned int, int>();

	cv::Mat cropped_image;
	screenshot(aa_bb).copyTo(cropped_image);

	std::vector<cv::Mat> channels(4);
	cv::split(cropped_image, channels);
	channels[0] = 0;
	channels[1] = 255 - channels[1];
	channels[2] = 255 - channels[2];
	channels[3] = 255;
	cv::merge(channels, cropped_image);

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/pop_popup.png", cropped_image);
#endif //SHOW_CV_DEBUG_IMAGE_VIEW

	std::vector<std::pair<std::string, cv::Rect>> ocr_result;
	try {
		ocr_result = detect_words(cropped_image);
	}
	catch (const std::exception& e) {
		std::cout << "there was an exception: " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "unknown exception" << std::endl;
	}

	return get_anno_population_from_ocr_result(ocr_result, cropped_image);
}

std::map<unsigned int, int> image_recognition::get_population_amount_from_statistic_screen() const
{
	const cv::Mat& im = screenshot;

	std::map<unsigned int, int> result;

	if (stats_screen.get_open_tab() != statistics_screen::tab::POPULATION)
		return result;

	cv::Mat roi = stats_screen.get_center_pane();

	if (roi.empty())
		return result;

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/statistics_window_scroll_area.png", roi);
#endif

	iterate_rows(roi, [&](const cv::Mat& row)
		{
			cv::Mat population_name = binarize(statistics_screen::get_cell(row, 0.076f, 0.2f));
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
			cv::imwrite("debug_images/population_name.png", population_name);
#endif
			std::vector<unsigned int> guids = get_guid_from_name(population_name, get_dictionary().population_levels);
			if (guids.size() != 1)
				return;

			cv::Mat text_img = binarize(statistics_screen::get_cell(row, 0.5f, 0.27f, 0.4f));
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
			cv::imwrite("debug_images/pop_amount_text.png", text_img);
#endif

			std::string number_string;

			for (const auto& mode : { tesseract::PSM_SINGLE_LINE, tesseract::PSM_SINGLE_WORD, tesseract::PSM_RAW_LINE })
			{
				std::vector<std::pair<std::string, cv::Rect>> texts = detect_words(text_img, mode);
				std::string joined_string = join(texts);


#ifdef CONSOLE_DEBUG_OUTPUT
				try {
					std::cout << get_dictionary().population_levels.at(guids.front()) << "\t" << joined_string << std::endl;
				}
				catch (...) {}
#endif

				std::vector<std::string> split_string;
				boost::split(split_string, joined_string, [](char c) {return c == '/' || c == '[' || c == '(' || c == '{'; });


				if (split_string.size() == 2 && std::regex_match(split_string.front(), std::regex("(\\d|\\s|[,.;:'])+")))
					number_string = split_string.front();
				else if ((texts.size() == 2 || texts.size() == 3 && texts[1].first.size() == 1) &&
					std::regex_match(texts.front().first, std::regex("(\\d|\\s|[,.;:'])+")))
					number_string = texts.front().first;
				
				if (!number_string.empty())
					break;
			}


			int population = number_from_string(number_string);

			if (population >= 0)
				result.emplace(guids.front(), population);

		});

	for (const auto& entry : get_dictionary().population_levels)
	{
		if (result.find(entry.first) == result.end())
			result[entry.first] = 0;
	}
	return result;
}

std::map<unsigned int, int> image_recognition::get_population_existing_buildings_from_statistic_screen() const
{
	const cv::Mat& im = screenshot;

	std::map<unsigned int, int> result;

	if (stats_screen.get_open_tab() != statistics_screen::tab::POPULATION)
		return result;

	cv::Mat roi = stats_screen.get_center_pane();

	if (roi.empty())
		return result;

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/statistics_window_scroll_area.png", roi);
#endif

	iterate_rows(roi, [&](const cv::Mat& row)
		{
			cv::Mat population_name = binarize(statistics_screen::get_cell(row, 0.076f, 0.2f));
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
			cv::imwrite("debug_images/population_name.png", population_name);
#endif
			std::vector<unsigned int> guids = get_guid_from_name(population_name, get_dictionary().population_levels);
			if (guids.size() != 1)
				return;

			cv::Mat text_img = binarize(statistics_screen::get_cell(row, 0.3f, 0.15f, 0.4f));
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
			cv::imwrite("debug_images/pop_houses_text.png", text_img);
#endif
			int houses = number_from_region(text_img);

			if (houses >= 0)
				result.emplace(guids.front(), houses);

		});

	for (const auto& entry : get_dictionary().population_levels)
	{
		if (result.find(entry.first) == result.end())
			result[entry.first] = 0;
	}
	return result;
}



std::map<unsigned int, int> image_recognition::get_population_workforce_from_statistic_screen() const
{
	const cv::Mat& im = screenshot;

	std::map<unsigned int, int> result;

	if (stats_screen.get_open_tab() != statistics_screen::tab::POPULATION)
		return result;

	cv::Mat roi = stats_screen.get_center_pane();

	if (roi.empty())
		return result;

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/statistics_window_scroll_area.png", roi);
#endif

	iterate_rows(roi, [&](const cv::Mat& row)
		{
			cv::Mat population_name = binarize(statistics_screen::get_cell(row, 0.076f, 0.2f));
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
			cv::imwrite("debug_images/population_name.png", population_name);
#endif
			std::vector<unsigned int> guids = get_guid_from_name(population_name, get_dictionary().population_levels);
			if (guids.size() != 1)
				return;

			cv::Mat text_img = binarize(statistics_screen::get_cell(row, 0.8f, 0.1f));
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
			cv::imwrite("debug_images/pop_houses_text.png", text_img);
#endif
			int workforce = number_from_region(text_img);

			if (workforce >= 0)
				result.emplace(guids.front(), workforce);

		});

	for (const auto& entry : get_dictionary().population_levels)
	{
		if (result.find(entry.first) == result.end())
			result[entry.first] = 0;
	}
	return result;
}

std::string image_recognition::get_selected_island()
{
	if (!selected_island.empty())
		return selected_island;

#ifdef CONSOLE_DEBUG_OUTPUT
	std::cout << "Island:\t";
#endif
	if (stats_screen.is_open())
	{


		if (stats_screen.is_all_islands_selected())
		{
#ifdef CONSOLE_DEBUG_OUTPUT
			std::cout << ALL_ISLANDS << std::endl;
#endif
			selected_session = SESSION_META;
			selected_island = ALL_ISLANDS;
			return ALL_ISLANDS;
		}

		std::string result;

		cv::Mat roi = binarize(stats_screen.get_center_header());

		if (roi.empty())
			return result;

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
		cv::imwrite("debug_images/header.png", roi);
#endif

		auto words_and_boxes = detect_words(roi, tesseract::PSM_SINGLE_LINE);
		result = join(words_and_boxes, true);

		
		auto island = stats_screen.get_island_from_list(result);
		selected_island = island.first;
		selected_session = island.second;


#ifdef CONSOLE_DEBUG_OUTPUT
		std::cout << result << std::endl;
#endif
		return selected_island;
	}
	else // statistics screen not open
	{
		cv::Rect pop_icon_position = find_population_icon();
		if (pop_icon_position.tl().x <= 0)
			return std::string();

		if (pop_icon_position.tl().x < 0.3 * screenshot.cols)
		{
#ifdef CONSOLE_DEBUG_OUTPUT
			std::cout << ALL_ISLANDS << std::endl;
#endif
			selected_session = SESSION_META;
			selected_island = ALL_ISLANDS;
			return ALL_ISLANDS;
		}
		else
		{
			cv::Mat island_name_img = screenshot(cv::Rect(0.0036f * screenshot.cols, 0.6641f * screenshot.rows, 0.115f * screenshot.cols, 0.0245f * screenshot.rows));
			island_name_img = binarize(island_name_img, true);

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
			cv::imwrite("debug_images/island_name_minimap.png", island_name_img);
#endif
			std::string result = join(detect_words(island_name_img, tesseract::PSM_SINGLE_LINE), true);

#ifdef CONSOLE_DEBUG_OUTPUT
			std::cout << result << std::endl;
#endif
			auto island = stats_screen.get_island_from_list(result);
			selected_island = island.first;
			selected_session = island.second;

			return selected_island;
		}

	}

	return std::string();
}

unsigned int image_recognition::get_selected_session()
{
	get_selected_island(); // update island information
	return selected_session;
}

const std::string image_recognition::ALL_ISLANDS = std::string("All Islands");

std::map<std::string, unsigned int> image_recognition::get_islands() const
{
	return stats_screen.island_to_session;
}

cv::Mat image_recognition::load_image(const std::string& path)
{
	cv::Mat img = cv::imread(path, cv::IMREAD_UNCHANGED);
	if (img.size().area() < 1) {
		throw std::invalid_argument("failed to load " + path);
	}
	if(!img.empty())
		cv::cvtColor(img, img, cv::COLOR_BGR2BGRA);

	return img;
}

cv::Mat image_recognition::binarize(cv::InputArray input, bool invert)
{
	if (input.empty())
		return input.getMat();

	cv::Mat thresholded;
	cv::cvtColor(input, thresholded, cv::COLOR_BGRA2GRAY);
	cv::threshold(thresholded, thresholded, 128, 255, (invert ? cv::THRESH_BINARY_INV : cv::THRESH_BINARY)  | cv::THRESH_OTSU);
	cv::cvtColor(thresholded, thresholded, cv::COLOR_GRAY2RGBA);

	return thresholded;
}



cv::Mat image_recognition::binarize_icon(cv::InputArray input, cv::Size target_size)
{
	if (input.empty())
		return input.getMat();

	cv::Mat thresholded;
	cv::Mat input_alpha_applied = blend_icon(input, cv::Scalar(0, 0, 0));
	cv::cvtColor(input_alpha_applied, thresholded, cv::COLOR_BGRA2GRAY);
	cv::threshold(thresholded, thresholded, 128, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
	if (thresholded.at<unsigned char>(thresholded.rows - 1, thresholded.cols-1) > 128)
		thresholded = 255 - thresholded;

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/icon_gray.png", thresholded);
#endif

	cv::Mat edge_image;
	cv::Canny(thresholded, edge_image, 20, 60, 3);
	std::vector < std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(edge_image, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_NONE);
	contours.erase(std::remove_if(
			contours.begin(), 
			contours.end(), 
			[](const std::vector<cv::Point>& e) {return e.size() < 10; }),
		contours.end());

	if (contours.empty())
		return cv::Mat(target_size, CV_8UC4);

	std::vector<cv::Point > contour_points;
	for (auto& contour : contours)
	{
		contour.push_back(contour.front());
		contour_points.insert(contour_points.end(), contour.begin(), contour.end());
	}
	cv::Rect roi = boundingRect(contour_points);

	cv::Mat output = thresholded(roi);

	if (target_size.width && target_size.height)
		cv::resize(output, output, target_size, 0, 0, cv::INTER_NEAREST);

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/icon_shilouette.png", output);
#endif

	return output;
}

cv::Mat image_recognition::convert_color_space_for_template_matching(cv::InputArray bgr_in)
{
	std::vector<cv::Mat> channels;
	cv::split(bgr_in, channels);
	for (auto& c : channels)
	{
		double min, max;
		cv::minMaxLoc(c, &min, &max);
		cv::threshold(c, c, 0.5f * (min + max), 255.f, cv::THRESH_BINARY);
	}
	cv::Mat ret;
	cv::merge(channels.data(), 3, ret);

	return ret;
}

cv::Mat image_recognition::gamma_invariant_hue_finlayson(cv::InputArray bgr_in)
{
	std::vector<cv::Mat> channels;
	cv::split(bgr_in.getMat(), channels);
	for (auto& c : channels) {
		c.convertTo(c, CV_32F);
		c = c / 255.f;
	}
	/*{
		std::ofstream of("pixel_red.csv", std::ios::out);
		for (int i = 0; i < channels[0].rows; i++)
			for (int j = 0; j < channels[0].cols; j++)
				of << channels[0].at<float>(i, j) << std::endl;

	}*/
	for (auto& c : channels) {
		cv::log(c, c);
	}
	cv::Mat ret[3];
	cv::divide((channels[2] - channels[1]), (channels[2] + channels[1] - 2.f * channels[0]), ret[0]);
	cv::divide((channels[0] - channels[2]), (channels[0] + channels[2] - 2.f * channels[1]), ret[1]);
	cv::divide((channels[1] - channels[0]), (channels[1] + channels[0] - 2.f * channels[2]), ret[2]);
	for (int i = 0; i < 3; i++) {
		/*{
			std::ofstream of("pixel_values_raw.csv", std::ios::out);
			for (int i = 0; i < ret.rows; i++)
				for (int j = 0; j < ret.cols; j++)
					of << ret.at<float>(i, j) << std::endl;

		}*/
		cv::threshold(ret[i], ret[i], 2.f, 1.f, cv::THRESH_TRUNC);
		ret[i] = -1.f * ret[i];
		cv::threshold(ret[i], ret[i], 2.f, 1.f, cv::THRESH_TRUNC);
		ret[i] = -1.f * ret[i];
		/*{
			std::ofstream of("pixel_values_pre_norm.csv", std::ios::out);
			for (int i = 0; i < ret[i].rows; i++)
				for (int j = 0; j < ret[i].cols; j++)
					of << ret[i].at<float>(i, j) << std::endl;

		}*/

		//cv::normalize(ret[i], ret[i], 0, 255, cv::NORM_MINMAX);
		ret[i] = ret[i] + 2.f;
		ret[i] = ret[i] / 4.f;
		ret[i] = ret[i] * 255.f;
		cv::imwrite("debug_images/ret" + std::to_string(i) + ".png", ret[i]);
		//H = (log(R)-log(G))/(log(R)+log(G)-2log(B))

		/*{
			std::ofstream of("pixel_values_post_norm.csv", std::ios::out);

			for (int i = 0; i < ret.rows; i++)
				for (int j = 0; j < ret.cols; j++)
					of << ret.at<float>(i, j) << std::endl;
		}*/
	}
#ifdef CONSOLE_DEBUG_OUTPUT
	std::cout << "done with gamma" << std::endl;
#endif
	return ret[0];
}

void image_recognition::write_image_per_channel(const std::string& path, cv::InputArray img)
{
	cv::Mat mat = img.getMat();
	std::vector<cv::Mat> channels;
	cv::split(mat, channels);
	for (int i = 0; i < channels.size(); i++) {
		cv::imwrite(path + "_" + std::to_string(i) + ".png", channels[i]);
	}
}

cv::Rect image_recognition::get_aa_bb(const std::list<cv::Point>& input)
{
	if (input.empty())
		return cv::Rect(cv::Point(0, 0), cv::Point(0, 0));

	cv::Point min(*input.begin());
	cv::Point max(*input.begin());

	for (const cv::Point& p : input) {
		min.x = std::min(min.x, p.x);
		min.y = std::min(min.y, p.y);
		max.x = std::max(max.x, p.x);
		max.y = std::max(max.y, p.y);
	}
	return cv::Rect(min, max + cv::Point(1, 1));
}

cv::Mat image_recognition::update(const std::string& language,
	const cv::Mat& img)
{
	selected_island = std::string();
	selected_session = 0;
	center_pane_selection = 0;
	population_icon_position = cv::Rect(-1,-1, 0, 0);

	if (has_language(language))
		this->language = language;
	else
		this->language = "english";

	update_ocr(this->language);

	if (img.empty())
		screenshot = take_screenshot();
	else
		screenshot = img;

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/screenshot.png", screenshot);
#endif

	stats_screen.update(screenshot);
	return screenshot;
}

cv::Mat image_recognition::take_screenshot()
{
	try{
	cv::Mat src;

	std::string window_name_regex_string("(Anno 1800)|(Anno 7)");
	std::regex window_name_regex(window_name_regex_string.data());

	HWND hwnd = NULL;
	int area = 0;
	struct lambda_parameter {
		HWND* hwnd_p;
		std::regex window_name_regex;
		int* area;
	} params{ &hwnd, window_name_regex, &area };

	EnumWindows(
		[](HWND local_hwnd, LPARAM lparam) {
			int length = GetWindowTextLength(local_hwnd);
			char* buffer = new char[length + 1];
			GetWindowText(local_hwnd, buffer, length + 1);
			std::string windowTitle(buffer);
			if (length == 0) {
				return TRUE;
			}
			if (std::regex_match(windowTitle, ((lambda_parameter*)lparam)->window_name_regex)) {
				RECT windowsize;    // get the height and width of the screen
				GetWindowRect(local_hwnd, &windowsize);

				int area = (windowsize.bottom - windowsize.top) * (windowsize.right - windowsize.left);
				if (*(((lambda_parameter*)lparam)->area))
				{
					std::cout << "WARNING: Multiple windows with title 'Anno 1800' detected. The server may not work because it captures the wrong one." << std::endl;
				}

				if (*(((lambda_parameter*)lparam)->area) < area)
				{
					*(((lambda_parameter*)lparam)->area) = area;
					*(((lambda_parameter*)lparam)->hwnd_p) = local_hwnd;
				}
			}
			return TRUE;
		}, (LPARAM)& params);

	if (hwnd == NULL)
	{
#ifdef CONSOLE_DEBUG_OUTPUT
		std::cout << "Can't find window with regex " << window_name_regex_string << std::endl
			<< "open windows are:" << std::endl;

		//print all open window titles
		EnumWindows([](HWND hWnd, LPARAM lparam) {
			int length = GetWindowTextLength(hWnd);
			char* buffer = new char[length + 1];
			GetWindowText(hWnd, buffer, length + 1);
			std::string windowTitle(buffer);
			std::cout << hWnd << ":  " << windowTitle << std::endl;
			if (length > 0)
			{
				std::cout << "match result "
					<< std::regex_match(windowTitle, ((lambda_parameter*)lparam)->window_name_regex) << std::endl;
			}
			return TRUE;
			}, (LPARAM)& params);
#else
		std::cout << "Anno 1800 window not found" << std::endl;
#endif

		return cv::Mat();
	}

	HDC hwindowDC = GetDC(NULL);
	HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

	RECT windowsize;    // get the height and width of the screen
	GetWindowRect(hwnd, &windowsize);

	const int height = windowsize.bottom - windowsize.top;  //change this to whatever size you want to resize to
	const int width = windowsize.right - windowsize.left;

	src.create(height, width, CV_8UC4);

	// create a bitmap
	HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, windowsize.right - windowsize.left, windowsize.bottom - windowsize.top);

	BITMAPINFOHEADER  bi;
	bi.biSize = sizeof(BITMAPINFOHEADER);    //http://msdn.microsoft.com/en-us/library/windows/window/dd183402%28v=vs.85%29.aspx
	bi.biWidth = width;
	bi.biHeight = -height;  //this is the line that makes it draw upside down or not
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	// use the previously created device context with the bitmap
	SelectObject(hwindowCompatibleDC, hbwindow);
	// copy from the window device context to the bitmap device context
	StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, windowsize.left, windowsize.top, width, height, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

	// avoid memory leak
	DeleteObject(hbwindow);
	DeleteDC(hwindowCompatibleDC);
	ReleaseDC(hwnd, hwindowDC);

	return src;
	}
	catch (...)
	{
		return cv::Mat();
	}
}

std::shared_ptr<tesseract::TessBaseAPI> image_recognition::ocr(nullptr);

std::vector<std::pair<std::string, cv::Rect>> image_recognition::detect_words(const cv::InputArray& in, const tesseract::PageSegMode mode)
{
	cv::Mat input = in.getMat();
	std::vector<std::pair<std::string, cv::Rect>> ret;

	try {
		const auto& cr = ocr;
		cr->SetPageSegMode(mode);

		// Set image data
		cr->SetImage(input.data, input.cols, input.rows, 4, input.step);

		cr->Recognize(0);
		tesseract::ResultIterator* ri = cr->GetIterator();
		tesseract::PageIteratorLevel level = tesseract::RIL_WORD;



		if (ri != 0) {
			ret.reserve(10);
			do {
				const char* word = ri->GetUTF8Text(level);
				float conf = ri->Confidence(level);
				int x1, y1, x2, y2;
				ri->BoundingBox(level, &x1, &y1, &x2, &y2);
//#ifdef CONSOLE_DEBUG_OUTPUT
//				printf("word: '%s';\t\tconf: %.2f; BoundingBox: %d,%d,%d,%d;\n",
//					word, conf, x1, y1, x2, y2);
//#endif
				std::string word_s = word ? std::string(word) : std::string();
				cv::Rect aa_bb(cv::Point(x1, y1), cv::Point(x2, y2));
				ret.push_back(std::make_pair(word_s, aa_bb));
				delete[] word;
			} while (ri->Next(level));
		}
	}
	catch (...) {}

	return ret;
}

std::map<unsigned int, int> image_recognition::get_anno_population_from_ocr_result(const std::vector<std::pair<std::string, cv::Rect>>& ocr_result, const cv::Mat& img) const
{
	const std::map<unsigned int, std::string>& dictionary = get_dictionary().population_levels;

	std::map<unsigned int, int> ret;

	typedef struct _match
	{
		float similarity;
		const std::pair<const unsigned int, std::string>* kw;
		const std::pair<std::string, cv::Rect>* occurrence;
	} match;

	std::priority_queue< match, std::vector<match>, auto(*)(const match&, const match&)->bool >  queue([](const match& lhs, const match& rhs) -> bool {
		return lhs.similarity < rhs.similarity;
		});

	int i = 0;
	for (const auto& pop_name_word : ocr_result)
	{
		for (const auto& kw : dictionary)
		{
			queue.emplace(match{
				lcs_length(pop_name_word.first, kw.second) / (float) std::max(pop_name_word.first.size(), kw.second.size()),
				&kw,
				&pop_name_word
				});
		}

		i++;
	}

	std::set<const std::pair<const unsigned int, std::string>*> matched_keywords;
	std::set<const std::pair<std::string, cv::Rect>*> matched_occurences;

	float similarity = 1.f;
	const float threshold = 0.66f;
	while (queue.size() && similarity > threshold)
	{
		const match m = queue.top(); queue.pop();
		similarity = m.similarity;

		if (similarity > threshold &&
			matched_keywords.find(m.kw) == matched_keywords.end() &&
			matched_occurences.find(m.occurrence) == matched_occurences.end())
		{

			matched_keywords.emplace(m.kw);
			matched_occurences.emplace(m.occurrence);

			std::string number_string;
			cv::Rect number_region;
			for (const auto& pop_value_word : ocr_result) 
			{
				if (*m.occurrence == pop_value_word)
					continue;
				if (std::abs(pop_value_word.second.tl().y - m.occurrence->second.tl().y) < 4
					&& pop_value_word.second.tl().x > m.occurrence->second.tl().x
					&& pop_value_word.first.find("0/") == std::string::npos) {
					number_string += std::regex_replace(pop_value_word.first, std::regex("\\D"), "");
					
					if (number_region.area()) // compute 
					{
						int max_x = number_region.x + number_region.width;
						int max_y = number_region.y + number_region.height;
						number_region.x = std::min(number_region.x, pop_value_word.second.x);
						number_region.y = std::min(number_region.y, pop_value_word.second.y);
						number_region.width = std::max(max_x, pop_value_word.second.x + pop_value_word.second.width) - number_region.x;
						number_region.height = std::max(max_y, pop_value_word.second.y + pop_value_word.second.height) - number_region.y;
					}
					else
					{
						number_region = pop_value_word.second;
					}

				}

			}

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
			cv::imwrite("debug_images/pop_number_region.png", img(number_region));
#endif
			number_region.x--;
			number_region.y--;
			number_region.height += 2;
			number_region.width += 2;

			int pop_from_string = number_from_string(number_string);
			int pop_from_region = number_from_region(img(number_region));

			int population = 0;
			if(std::floor(std::log10(pop_from_region)) <= std::floor(std::log10(pop_from_string)))
				population = pop_from_string;
			else
				population = std::max(number_from_string(number_string), number_from_region(img(number_region)));


			//if word based ocr fails, try symbols (probably only 1 digit population)
			if (population == 0) {
#ifdef CONSOLE_DEBUG_OUTPUT
				std::cout << "could not find population number, if the number is only 1 digit, this is a known problem" << std::endl;
#endif
			}


			if (population > 0) {
				auto insert_result = ret.insert(std::make_pair(m.kw->first, population));
#ifdef CONSOLE_DEBUG_OUTPUT
				std::cout << "new value for " << m.kw->first << ": " << population << std::endl;
#endif
			}
		}
	}

	if (!ret.size())
		return ret;

	for (auto& entry : dictionary)
	{
		if (ret.find(entry.first) == ret.end())
		{
			ret.emplace(entry.first, 0);
		}
	}

	return ret;
}

bool image_recognition::has_language(const std::string& language) const
{
	return dictionaries.find(language) != dictionaries.end() && tesseract_languages.find(language) != tesseract_languages.end();
}

const keyword_dictionary& image_recognition::get_dictionary() const
{
	auto iter = dictionaries.find(language);
	if (iter == dictionaries.end())
		return keyword_dictionary();
	return iter->second;
}

std::map<unsigned int, std::string>  image_recognition::make_dictionary(const std::vector<phrase>& list) const
{
	std::map<unsigned int, std::string> result;
	const std::map<unsigned int, std::string>& source = get_dictionary().ui_texts;

	for (phrase w : list)
	{
		result[(unsigned int)w] = source.at((unsigned int)w);
	}

	return result;
}

std::vector<double> image_recognition::get_hu_moments(cv::Mat img)
{
	cv::Moments moments = cv::moments(detect_edges(img));
	std::vector<double> hu_moments;
	cv::HuMoments(moments, hu_moments);

	return hu_moments;
}

cv::Mat image_recognition::detect_edges(const cv::Mat& im)
{
	cv::Mat canny, grey, edges;
	if (!im.empty())
	{
		cv::cvtColor(im, grey, cv::COLOR_BGRA2GRAY);
		cv::Canny(grey, edges, 30, 50);
	}

	return edges;
}

std::vector<int> image_recognition::find_horizontal_lines(const cv::Mat& im)
{
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/scroll_area.png", im);
#endif

	cv::Mat edges = detect_edges(im);

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/scroll_area_edges.png", edges);
#endif

	std::vector<cv::Vec4i> lines;
	cv::HoughLinesP(edges, lines, 2, CV_PI / 2, im.cols/2.f, im.cols*0.75f, im.cols * 0.15f);
	
	std::vector<int> hlines;
	for (auto& line : lines)
	{
		if (std::abs(line[1] - line[3]) <= 3 &&
			std::abs(line[2] - line[0]) > 0.8f*im.cols)
		{
			hlines.push_back(line[1]);
		}
	}
	std::sort(hlines.begin(), hlines.end());
	return hlines;
}

void image_recognition::iterate_rows(const cv::Mat& im,
												 const std::function<void(const cv::Mat& row)> f)
{
	std::vector<int> lines(find_horizontal_lines(im));

	if (!lines.size())
		return;

	std::vector<int> heights;
	int prev_hline = 0;
	for (int hline : lines)
	{
		int height = hline - prev_hline;
		if (height > 10)
		{
			heights.push_back(height);
		}
		prev_hline = hline;
	}

	if (heights.empty())
		return;

	prev_hline = 0;
 	int mean_row_height = heights[heights.size() / 2];
	int row_height = 0;

	for (auto hline = lines.begin(); hline != lines.end(); ++hline)
	{
		int height = *hline - prev_hline;
		if (height < 10)
		{
			prev_hline = *hline;
			continue;
		}

		auto next_hline = hline;
		while (height <= 0.9 * mean_row_height && next_hline != lines.end())
		{
			++next_hline;
			height = *next_hline - prev_hline;
		}

		if (height > 0.9 * mean_row_height && height < 1.1 * mean_row_height)
		{
			row_height = height;
			f(im(cv::Rect(0, prev_hline, im.cols, height)));
		}
		else
			next_hline = hline;

		prev_hline = *next_hline;
	}

	if (row_height)
	{
		int height = lines.back() + row_height < im.rows ? row_height : im.rows - lines.back();
		if (height > 10 && height > row_height * 0.95f)
			f(im(cv::Rect(0, lines.back(), im.cols, height)));
	}
}



std::map<unsigned int, int> image_recognition::get_optimal_productivities()
{
	const cv::Mat& im = screenshot;

	std::map<unsigned int, int> result;
	if (stats_screen.get_open_tab() != statistics_screen::tab::PRODUCTION)
		return result;

#ifdef CONSOLE_DEBUG_OUTPUT
	std::cout << "Optimal productivities" << std::endl;
#endif

	cv::Mat buildings_text = binarize(im(cv::Rect(0.6522f * im.cols, 0.373f * im.rows, 0.093f * im.cols, 0.0245f * im.rows)));
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/buildings_text.png", buildings_text);
#endif
	int buildings_count = number_from_region(buildings_text);
#ifdef CONSOLE_DEBUG_OUTPUT
	std::cout << std::endl;
#endif

	if (buildings_count < 0)
		return result;

	cv::Mat roi = stats_screen.get_right_pane();

	if (roi.empty())
		return result;

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/statistics_window_scroll_area.png", roi);
#endif


	cv::Mat factory_text = im(cv::Rect(0.6522f * im.cols, 0.373f * im.rows, 0.093f * im.cols, 0.0245f * im.rows));
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/factory_text.png", factory_text);
#endif
	std::vector<unsigned int> guids = get_guid_from_name(factory_text, get_dictionary().factories);
	if (guids.size() != 1)
		return result;

#ifdef CONSOLE_DEBUG_OUTPUT
	try {
		std::cout << get_dictionary().factories.at(guids.front()) << ":\t";
	}
	catch (...) {}
	std::cout << std::endl;
#endif

	std::vector<float> productivities;
	iterate_rows(roi, [&](const cv::Mat& row)
		{
			int productivity = 0;
			for (const std::pair<float, float> cell : std::vector<std::pair<float, float>>({ {0.6f, 0.2f}, {0.8f, 0.2f} }))
			{
				cv::Mat productivity_text = binarize(statistics_screen::get_cell(row, cell.first, cell.second));
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
				cv::imwrite("debug_images/productivity_text.png", productivity_text);
#endif
				int prod = number_from_region(productivity_text);
				if (prod > 1000) // sometimes '%' is detected as '0/0' or '00'
					prod /= 100;

				if (prod >= 0)
					productivity += prod;
				else
					return;
			}

#ifdef CONSOLE_DEBUG_OUTPUT
			std::cout << std::endl;
#endif

			productivities.push_back(productivity);

		});

	if (productivities.empty())
		return result;

	int sum = std::accumulate(productivities.begin(), productivities.end(), 0);

#ifdef CONSOLE_DEBUG_OUTPUT
	std::cout << " = " << sum << std::endl;
#endif

	if (productivities.size() != buildings_count)
	{
		std::sort(productivities.begin(), productivities.end());
		sum += (buildings_count - productivities.size()) * productivities[productivities.size() / 2];
	}

	result.emplace(guids.front(), sum / buildings_count);

	return result;
}

std::map<unsigned int, int> image_recognition::get_average_productivities()
{
	const cv::Mat& im = screenshot;

	std::map<unsigned int, int> result;
	if (stats_screen.get_open_tab() != statistics_screen::tab::PRODUCTION)
		return result;

	cv::Mat roi = stats_screen.get_center_pane();

	if (roi.empty())
		return result;

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/statistics_window_scroll_area.png", roi);
#endif

#ifdef CONSOLE_DEBUG_OUTPUT
	std::cout << "Average productivities" << std::endl;
#endif

	iterate_rows(roi, [&](const cv::Mat& row)
		{
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
			cv::imwrite("debug_images/row.png", row);
#endif

			cv::Mat product_icon = statistics_screen::get_square_region(row, statistics_screen::position_factory_icon);
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
			cv::imwrite("debug_images/factory_icon.png", product_icon);
#endif
			std::vector<unsigned int> p_guids = get_guid_from_icon(product_icon, product_icons);
			if (p_guids.empty())
				return;

#ifdef CONSOLE_DEBUG_OUTPUT
			try{
				std::cout << get_dictionary().products.at(p_guids.front()) << ":\t";
			}
			catch (...) {}
#endif

			bool selected = stats_screen.is_selected(row.at<cv::Vec4b>(0.1f * row.rows, 0.5f * row.cols));
			cv::Mat productivity_text = binarize(statistics_screen::get_cell(row, 0.7f, 0.1f, 0.4f), selected);
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
			cv::imwrite("debug_images/productivity_text.png", productivity_text);
#endif
			int prod = number_from_region(productivity_text);

			if (prod > 500)
				prod /= 100;

			if (prod >= 0)
			{
				for(unsigned int p_guid : p_guids)
					for(unsigned int f_guid: product_to_factories[p_guid])
						result.emplace(f_guid, prod);
			}


#ifdef CONSOLE_DEBUG_OUTPUT
			std::cout << std::endl;
#endif

		});


	return result;
}

std::map<unsigned int, int> image_recognition::get_assets_existing_buildings()
{
	switch (stats_screen.get_open_tab())
	{
	case statistics_screen::tab::FINANCE:
		return get_assets_existing_buildings_from_finance_screen();
	case statistics_screen::tab::PRODUCTION:
		return get_factories_existing_buildings_from_production_screen();
	case statistics_screen::tab::POPULATION:
		return get_population_existing_buildings_from_statistic_screen();
	}
	return std::map<unsigned int, int>();
}

std::map<unsigned int, int> image_recognition::get_assets_existing_buildings_from_finance_screen()
{
	std::map<unsigned int, int> result;

	if (stats_screen.get_open_tab() != statistics_screen::tab::FINANCE)
		return result;

	cv::Mat roi = stats_screen.get_center_pane();

	if (roi.empty())
		return result;

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/statistics_window_scroll_area.png", roi);
#endif
	
	std::map<unsigned int, std::string> category_dict = make_dictionary({ phrase::RESIDENTS, phrase::PRODUCTION });

	cv::Mat text = binarize(stats_screen.get_right_header());

	std::vector<unsigned int> selection = get_guid_from_name(text, category_dict);



#ifdef CONSOLE_DEBUG_OUTPUT
	try{
		std::cout << get_dictionary().ui_texts.at(center_pane_selection) << std::endl;
	}
	catch (...) {}
#endif

	if (selection.size() != 1)
		return result; // nothing selected that we want to evaluate

	center_pane_selection = selection.front();

	const std::map<unsigned int, std::string>* dictionary = nullptr;
	switch ((phrase) center_pane_selection)
	{
	case phrase::RESIDENTS:
		dictionary = &get_dictionary().population_levels;
		break;
	case phrase::PRODUCTION:
		dictionary = &get_dictionary().factories;
		break;
	}

	roi = stats_screen.get_right_pane();
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("debug_images/statistics_window_table_area.png", roi);
#endif
	std::vector<unsigned int> prev_guids;
	int prev_count = 0;

	iterate_rows(roi, [&](const cv::Mat& row)
		{
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
		cv::imwrite("debug_images/row.png", row);
		cv::imwrite("debug_images/selection_test.png", row(cv::Rect((int)(0.037f * row.cols), (int)(0.5f * row.rows), 10, 10)));
#endif
			bool is_summary_entry = stats_screen.closer_to(row.at<cv::Vec4b>( 0.5f * row.rows, 0.037f * row.cols), statistics_screen::expansion_arrow, statistics_screen::background_brown_light);

			if (is_summary_entry)
			{
				prev_guids.clear();

				cv::Mat text = binarize(statistics_screen::get_cell(row, 0.15f , 0.5f ));

				std::vector<unsigned int> guids = get_guid_from_name(text, *dictionary);

			
#ifdef CONSOLE_DEBUG_OUTPUT
				try{
					for (unsigned int guid : guids)
						std::cout << dictionary->at(guid) << ", ";
					std::cout << "\t";
				}
				catch (...) {}
#endif

				cv::Mat count_text = binarize(statistics_screen::get_cell(row, 0.15f, 0.5f));

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
				cv::imwrite("debug_images/count_text.png", count_text);
#endif

				std::vector<std::pair<std::string, cv::Rect>> words = detect_words(count_text, tesseract::PSM_SINGLE_LINE);
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
#ifdef CONSOLE_DEBUG_OUTPUT
				std::cout << number_string;
#endif
				int count = std::numeric_limits<int>::lowest();
				try { count = std::stoi(number_string); }
				catch (...) {
#ifdef CONSOLE_DEBUG_OUTPUT
					std::cout << " (could not recognize number)";
#endif
				}

				if (count >= 0)
				{
					if (guids.size() != 1 && get_selected_session() && !stats_screen.is_all_islands_selected())
						filter_factories(guids, get_selected_session());

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
				cv::Mat session_icon = stats_screen.get_cell(row, 0.08f, 0.08f);
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
				cv::imwrite("debug_images/session_icon.png", session_icon);
#endif

				unsigned int session_guid = get_session_guid(session_icon);
				if (!session_guid)
				{
					// prev_guids.clear();
					return;
				}

				filter_factories(prev_guids, session_guid);

				if (prev_guids.size() == 1)
					result.emplace(prev_guids.front(), prev_count);
				
					prev_guids.clear();
				
			}
#ifdef CONSOLE_DEBUG_OUTPUT
			std::cout << std::endl;
#endif
		});

	return result;
}

std::map<unsigned int, int> image_recognition::get_factories_existing_buildings_from_production_screen()
{
	return std::map<unsigned int, int>();
}


int image_recognition::number_from_region(const cv::Mat& im) const
{
	std::string number_string = join(detect_words(im, tesseract::PageSegMode::PSM_SINGLE_LINE));

#ifdef CONSOLE_DEBUG_OUTPUT
	std::cout << number_string << "\t";
#endif

	return number_from_string(number_string);
}

int image_recognition::number_from_string(const std::string& word) const
{
	std::string number_string = word;
	for (char& c : number_string)
	{
		auto iter = letter_to_digit.find(c);
		if (iter != letter_to_digit.end())
			c = iter->second;
	}

	number_string = std::regex_replace(number_string, std::regex("\\D"), "");

	try { return std::stoi(number_string); }
	catch (...) {
#ifdef CONSOLE_DEBUG_OUTPUT
		std::cout << "could not match number string: " << number_string << std::endl;
#endif
	}
	return std::numeric_limits<int>::lowest();
}






// Function to find length of Longest Common Subsequence of substring
// X[0..m-1] and Y[0..n-1]
// From https://www.techiedelight.com/longest-common-subsequence/
int image_recognition::lcs_length(std::string X, std::string Y)
{
	int m = X.length(), n = Y.length();

	// lookup table stores solution to already computed sub-problems
	// i.e. lookup[i][j] stores the length of LCS of substring
	// X[0..i-1] and Y[0..j-1]
	std::vector<std::vector<int>> lookup;
	lookup.resize(m + 1);
	for (auto& row : lookup)
		row.resize(n + 1);

	// first column of the lookup table will be all 0
	for (int i = 0; i <= m; i++)
		lookup[i][0] = 0;

	// first row of the lookup table will be all 0
	for (int j = 0; j <= n; j++)
		lookup[0][j] = 0;

	// fill the lookup table in bottom-up manner
	for (int i = 1; i <= m; i++)
	{
		for (int j = 1; j <= n; j++)
		{
			// if current character of X and Y matches
			if (X[i - 1] == Y[j - 1])
				lookup[i][j] = lookup[i - 1][j - 1] + 1;

			// else if current character of X and Y don't match
			else
				lookup[i][j] = std::max(lookup[i - 1][j], lookup[i][j - 1]);
		}
	}

	// LCS will be last entry in the lookup table
	return lookup[m][n];
}

std::string image_recognition::join(const std::vector<std::pair<std::string, cv::Rect>>& words, bool insert_spaces) const
{
	std::string result;

	for (const auto& pair : words)
	{
		result += pair.first;
		if(insert_spaces)
			result += " ";
	}

	if (insert_spaces)
		result.pop_back();

	return result;
}

void image_recognition::update_ocr(const std::string& language)
{
#ifdef CONSOLE_DEBUG_OUTPUT
	std::cout << "Update tesseract." << std::endl;
#endif

	const char* lang = tesseract_languages.find(language)->second.c_str();
//	if(ocr_)
//		std::cout << lang << " -> " << ocr_->GetInitLanguagesAsString() << " (equal: " << strcmp(ocr_->GetInitLanguagesAsString(), lang) << ")" << std::endl;
	if (!ocr || strcmp(ocr->GetInitLanguagesAsString(), lang)) {
		ocr.reset(new tesseract::TessBaseAPI());
		
		GenericVector<STRING> keys;
		GenericVector<STRING> values;

		keys.push_back("user_defined_dpi");
		values.push_back("70");

	/*	keys.push_back("textord_min_xheight"); values.push_back("8");
		keys.push_back("stopper_smallword_size"); values.push_back("1");
		keys.push_back("quality_min_initial_alphas_reqd"); values.push_back("1");
		keys.push_back("tessedit_preserve_min_wd_len"); values.push_back("1");

		keys.push_back("language_model_penalty_non_dict_word"); values.push_back("1");
		keys.push_back("segment_penalty_dict_nonword"); values.push_back("10");*/
		//keys.push_back("load_system_dawg"); values.push_back("F");
		//keys.push_back("load_freq_dawg"); values.push_back("F");
		//keys.push_back("user_words_suffix"); values.push_back((std::string(lang) + std::string(".user-words.txt")).c_str());
		//keys.push_back("user_patterns_suffix"); values.push_back((std::string(lang) + std::string(".user-patterns.txt")).c_str());
		
		if (ocr->Init(NULL, lang ? lang : "eng", tesseract::OEM_DEFAULT, nullptr, 0, &keys, &values, false))
		{
			std::cout << "error initialising tesseract" << std::endl;
		}
		
//		ocr_->SetVariable("CONFIGFILE", "bazaar");

	}

}

const std::map<std::string, std::string> image_recognition::tesseract_languages = {
	{"english", "eng"},
	{"chinese", "chi_sim"},
	{"polish", "pol"},
	{"russian", "rus"},
	{"french", "fra"},
	{"korean", "kor"},
	{"japanese", "jpn"},
	{"italian", "ita"},
	{"german", "deu"},
	{"spanish", "spa"}
};


