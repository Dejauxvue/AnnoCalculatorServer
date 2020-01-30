#pragma once

#include "reader_util.hpp"

namespace reader
{

class hud_statistics
{
public:
	hud_statistics(image_recognition& recog);

	void update(const std::string& language,
		const cv::Mat& img);

	/*
* Searches for population icon of of tooltip of HUD
*/
	cv::Rect find_population_icon();

	/**
* from the detected words [ocr_result] with thei bounding boxes (unused)
*
* returna map with entries for all detected population types referred by their english name
*/
	std::map<unsigned int, int> get_anno_population_from_ocr_result(
		const std::vector<std::pair<std::string, cv::Rect>>& ocr_result, const cv::Mat& img) const;

	/**
*
* returns a map with entries for all detected population types referred by their GUID
*/
	std::map < unsigned int, int> get_population_amount();

	std::string get_selected_island();

private:
	image_recognition& recog;
	std::string selected_island;
	// -1 if not searched for, 0 if not found
	cv::Rect population_icon_position;
};
}