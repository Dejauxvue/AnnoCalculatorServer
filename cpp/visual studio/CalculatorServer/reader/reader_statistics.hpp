#pragma once

#include <string>
#include <map>

#include "reader_statistics_screen.hpp"
#include "reader_hud_statistics.hpp"

namespace reader
{

class statistics 
{
public:
	statistics(image_recognition& recog);

	void update(const std::string& language, const cv::Mat& img);

	/**
*
* returns a map with entries for all detected population types referred by their GUID
*/
	std::map < unsigned int, int> get_population_amount();



	/*
* Returns count of existing buildings (houses/factories).
* Returns an empty map in case no information is found.
*/
	std::map < unsigned int, int> get_assets_existing_buildings();

	/*
* Returns percentile productivity for factories.
* Returns an empty map in case no information is found.
*/
	std::map < unsigned int, int> get_average_productivities();
	std::map < unsigned int, int> get_optimal_productivities();

	std::string get_selected_island();
	std::map<std::string, unsigned int> get_islands();
	const keyword_dictionary& get_dictionary();
	bool has_language(const std::string& language);

private:
	image_recognition& recog;
	statistics_screen stats_screen;
	hud_statistics hud;

};
}