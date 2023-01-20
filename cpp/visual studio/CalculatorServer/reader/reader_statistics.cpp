#include "reader_statistics.hpp"

#include <iostream>

namespace reader
{
	
statistics::statistics(image_recognition& recog)
	:
	recog(recog),
	stats_screen(recog)
	//hud(recog)
{
}

void statistics::update(const std::string& language, const cv::Mat& img)
{
	stats_screen.update(language, img);
	//hud.update(language, img);
}



std::map<unsigned int, statistics_screen::properties> statistics::get_all()
{
	using properties = statistics_screen::properties;
	
	std::map<unsigned int, properties> result;
	
	if (stats_screen.is_open())
		switch(stats_screen.get_open_tab())
		{
		case statistics_screen::tab::PRODUCTION:
			result = stats_screen.get_factory_properties();
			break;

		case statistics_screen::tab::FINANCE:
			for (const auto& entry : stats_screen.get_assets_existing_buildings_from_finance_screen())
				result.emplace(entry.first, properties({
				{statistics_screen::KEY_EXISTING_BUILDINGS, entry.second}
					}));
			break;
			
		case statistics_screen::tab::POPULATION:
			result = stats_screen.get_population_properties();
			
		case statistics_screen::tab::STORAGE:
		case statistics_screen::tab::ITEMS:
			break;
		}
	//else
	//	for (const auto& entry : hud.get_population_amount())
	//		result.emplace(entry.first, properties({
	//		{statistics_screen::KEY_AMOUNT, entry.second}
	//			}));

	return result;
}

std::pair<unsigned int, int> statistics::get_optimal_productivity()
{
	return stats_screen.get_optimal_productivity();
}

std::string statistics::get_selected_island()
{


	if (recog.is_verbose()) {
		std::cout << "Island:\t";
	}
	if (stats_screen.is_open())
	{
		return stats_screen.get_selected_island();

	}
	//else // statistics screen not open
	//{
	//	/*std::string name = hud.get_selected_island();
	//	auto island = stats_screen.get_island_from_list(name);
	//	return island.first;*/
	//	return hud.get_selected_island();
	//}

	return std::string();
}

std::map<std::string, unsigned int> statistics::get_islands() const
{
	return stats_screen.get_islands();
}

std::map<std::string, unsigned int> statistics::get_current_islands() const
{
	return stats_screen.get_current_islands();
}

const keyword_dictionary& statistics::get_dictionary()
{
	return recog.get_dictionary();
}

bool statistics::has_language(const std::string& language)
{
	return recog.has_language(language);
}

}
