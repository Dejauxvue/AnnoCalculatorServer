#include "reader_statistics.hpp"

namespace reader
{
reader::reader()
	:
	recog(),
	stats_screen(recog),
	hud(recog)
{
}

void reader::update(const std::string& language, const cv::Mat& img)
{
	stats_screen.update(language, img);
	hud.update(language, img);
}

std::map<unsigned int, int> reader::get_population_amount()
{
	if (stats_screen.is_open())
		return stats_screen.get_population_amount();
	else
		return hud.get_population_amount();
}

std::map<unsigned int, int> reader::get_average_productivities()
{
	return stats_screen.get_average_productivities();
}

std::map<unsigned int, int> reader::get_optimal_productivities()
{
	return stats_screen.get_optimal_productivities();
}

std::string reader::get_selected_island()
{


#ifdef CONSOLE_DEBUG_OUTPUT
	std::cout << "Island:\t";
#endif
	if (stats_screen.is_open())
	{
		return stats_screen.get_selected_island();

	}
	else // statistics screen not open
	{
		std::string name = hud.get_selected_island();
		auto island = stats_screen.get_island_from_list(name);
		return island.first;

	}

	return std::string();
}

std::map<std::string, unsigned int> reader::get_islands()
{
	return stats_screen.get_islands();
}

const keyword_dictionary& reader::get_dictionary()
{
	return recog.get_dictionary();
}

bool reader::has_language(const std::string& language)
{
	return recog.has_language(language);
}

std::map<unsigned int, int> reader::get_assets_existing_buildings()
{
	return stats_screen.get_assets_existing_buildings();
}

}
