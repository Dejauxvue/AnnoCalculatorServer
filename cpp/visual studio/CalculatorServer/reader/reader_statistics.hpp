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
* Returns the information gathered for factories and population levels referred by their GUID
*/
	std::map <unsigned int, statistics_screen::properties> get_all();


	/*
* Returns percentile productivity for factories.
* Returns an empty map in case no information is found.
*/
	std::pair < unsigned int, int> get_optimal_productivity();


	std::string get_selected_island();
	std::map<std::string, unsigned int> get_islands() const;
	std::map<std::string, unsigned int> get_current_islands() const;
	
	const keyword_dictionary& get_dictionary();
	bool has_language(const std::string& language);

private:
	image_recognition& recog;
	statistics_screen stats_screen;
	hud_statistics hud;

};
}