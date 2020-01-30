#include "reader_hud_statistics.hpp"

#include <regex>

namespace reader
{
	hud_statistics::hud_statistics(image_recognition& recog)
		:
		recog(recog),
		population_icon_position(cv::Rect(-1, -1, 0, 0))
	{}

	void hud_statistics::update(const std::string& language,
		const cv::Mat& img)
	{
		population_icon_position = cv::Rect(-1, -1, 0, 0);
		selected_island.clear();
		recog.update(language, img);
	}

	cv::Rect hud_statistics::find_population_icon()
	{
		if (population_icon_position.tl().x >= 0)
			return population_icon_position;

		const auto fit_criterion = [](float e) {return e < 20000.f; };

		struct template_images {
			std::string resolution_id = "uninitialized";
			cv::Mat island_pop_symbol;
		};

		static template_images templates;
		cv::Mat screenshot = recog.get_screenshot();
		std::string resolution_id = std::to_string(screenshot.cols) + "x" + std::to_string(screenshot.rows);
		if (templates.resolution_id != resolution_id)
		{
			try {
#ifdef CONSOLE_DEBUG_OUTPUT
				std::cout << "detected resolution: " << resolution_id
					<< std::endl;
#endif
				templates.island_pop_symbol = recog.load_image("image_recon/" + resolution_id + "/population_symbol_with_bar.bmp");
				templates.resolution_id = resolution_id;
			}
			catch (const std::invalid_argument & e) {
				std::cout << e.what() << ". Make sure the Anno 1800 is focused and has proper resolution!" << std::endl;
				population_icon_position = cv::Rect(0, 0, 0, 0);
				return population_icon_position;
			}
		}


		cv::Mat im_copy = screenshot(cv::Rect(cv::Point(0, 0), cv::Size(screenshot.cols, screenshot.rows / 2)));

		const auto pop_symbol_match_result = recog.match_template(im_copy, templates.island_pop_symbol);

		if (!fit_criterion(pop_symbol_match_result.second)) {
#ifdef CONSOLE_DEBUG_OUTPUT
			std::cout << "can't find population" << std::endl;
#endif
			population_icon_position = cv::Rect(0, 0, 0, 0);
		}
		population_icon_position = pop_symbol_match_result.first;

		return population_icon_position;
	}

	std::map<unsigned int, int> hud_statistics::get_anno_population_from_ocr_result(const std::vector<std::pair<std::string, cv::Rect>>& ocr_result, const cv::Mat& img) const
	{
		const std::map<unsigned int, std::string>& dictionary = recog.get_dictionary().population_levels;

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
					recog.lcs_length(pop_name_word.first, kw.second) / (float)std::max(pop_name_word.first.size(), kw.second.size()),
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

			if (similarity > threshold&&
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

				int pop_from_string = recog.number_from_string(number_string);
				int pop_from_region = recog.number_from_region(img(number_region));

				int population = 0;
				if (std::floor(std::log10(pop_from_region)) <= std::floor(std::log10(pop_from_string)))
					population = pop_from_string;
				else
					population = std::max(recog.number_from_string(number_string), recog.number_from_region(img(number_region)));


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

	std::map<unsigned int, int> hud_statistics::get_population_amount()
	{
		cv::Rect pop_icon_position = find_population_icon();

		if (pop_icon_position.tl().x <= 0)
			return std::map<unsigned int, int>();

		auto region = recog.find_rgb_region(recog.get_screenshot(), pop_icon_position.br(), 0);
		cv::Rect aa_bb = recog.get_aa_bb(region);
		if (aa_bb.area() <= 0)
			return std::map<unsigned int, int>();

		cv::Mat cropped_image;
		recog.get_screenshot()(aa_bb).copyTo(cropped_image);

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
			ocr_result = recog.detect_words(cropped_image);
		}
		catch (const std::exception & e) {
			std::cout << "there was an exception: " << e.what() << std::endl;
		}
		catch (...)
		{
			std::cout << "unknown exception" << std::endl;
		}

		return get_anno_population_from_ocr_result(ocr_result, cropped_image);
	}

	std::string hud_statistics::get_selected_island()
	{
		if (!selected_island.empty())
			return selected_island;

		cv::Rect pop_icon_position = find_population_icon();
		if (pop_icon_position.tl().x <= 0)
			return std::string();

		if (pop_icon_position.tl().x < 0.3 * recog.get_screenshot().cols)
		{
#ifdef CONSOLE_DEBUG_OUTPUT
			std::cout << ALL_ISLANDS << std::endl;
#endif
			selected_island = recog.ALL_ISLANDS;
			return recog.ALL_ISLANDS;
		}
		else
		{
			cv::Mat screenshot = recog.get_screenshot();

			cv::Mat island_name_img = screenshot(cv::Rect(0.0036f * screenshot.cols, 0.6641f * screenshot.rows, 0.115f * screenshot.cols, 0.0245f * screenshot.rows));
			island_name_img = recog.binarize(island_name_img, true);

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
			cv::imwrite("debug_images/island_name_minimap.png", island_name_img);
#endif
			std::string result = recog.join(recog.detect_words(island_name_img, tesseract::PSM_SINGLE_LINE), true);

#ifdef CONSOLE_DEBUG_OUTPUT
			std::cout << result << std::endl;
#endif
			selected_island = result;

			return result;
		}
	}



}