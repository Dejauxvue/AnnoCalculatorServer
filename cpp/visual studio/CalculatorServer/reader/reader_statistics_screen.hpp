#pragma once

#include "reader_util.hpp"

namespace reader
{

class statistics_screen_params
{
public:
	static const std::string all_islands;
	static const cv::Scalar background_brown_light;
	static const cv::Scalar background_blue_dark;
	static const cv::Scalar foreground_brown_light;
	static const cv::Scalar foreground_brown_dark;
	static const cv::Scalar expansion_arrow;

	static const cv::Rect2f position_factory_icon;
	static const cv::Rect2f position_small_factory_icon;
	static const cv::Rect2f position_population_icon;

	/* pane rectangles relative to [0,1]² image */
	static const cv::Rect2f pane_tabs;
	static const cv::Rect2f pane_title;
	static const cv::Rect2f pane_all_islands;
	static const cv::Rect2f pane_islands;
	static const cv::Rect2f pane_finance_center;
	static const cv::Rect2f pane_finance_right;
	static const cv::Rect2f pane_production_center;
	static const cv::Rect2f pane_production_right;
	static const cv::Rect2f pane_population_center;
	static const cv::Rect2f pane_header_center;
	static const cv::Rect2f pane_header_right;
};

/*
* Stores resolution independent properties of the statistics menu
* Allows to perform elementary boolean tests
* use update() to pass a new screenshot
*/
class statistics_screen
{
public:
	enum class tab
	{
		NONE = 0,
		FINANCE = 3,
		PRODUCTION = 1,
		POPULATION = 4,
		STORAGE = 2,
		ITEMS = 5
	};



	statistics_screen(image_recognition& recog);

	void update(const std::string& language, const cv::Mat& img);

	

	/*
	* Accessor functions to read data
	**/
	std::map < unsigned int, int> get_population_amount() const;

	/*
	* Returns percentile productivity for factories.
	* Returns an empty map in case no information is found.
	*/
	std::map < unsigned int, int> get_average_productivities();
	std::map < unsigned int, int> get_optimal_productivities();

	/*
	* Returns count of existing buildings (houses/factories).
	* Returns an empty map in case no information is found.
	*/
	std::map < unsigned int, int> get_assets_existing_buildings();
	std::map < unsigned int, int> get_assets_existing_buildings_from_finance_screen();
	std::map<unsigned int, int> get_population_existing_buildings() const;

	std::map<unsigned int, int> get_population_workforce() const;

	/*
* Returns the name of the selected island in the left pane of
* the statistics screen
* Returns ALL_ISLANDS
*/
	std::string get_selected_island();
	unsigned int get_selected_session();


	/*
* Returns all islands seen in the statistics screen so far
* Returns island name and associated session
*/
	std::map<std::string, unsigned int> get_islands() const;

	bool is_open() const;

	tab get_open_tab() const;

	bool is_all_islands_selected() const;

	/*
	* Returns the island (name, session) from the stored @ref{island_to_session} that matches @param{name} (some differences allowed)
	* In case no match is found, (@param{name}, 0) is returned.
	*/
	std::pair<std::string, unsigned int> get_island_from_list(std::string name) const;

	/*
	* Returns the panes dependent on the opened tab
	* Empty image, if !is_open or no position info for pane
	*/
	cv::Mat get_center_pane() const;
	cv::Mat get_left_pane() const;
	cv::Mat get_right_pane() const;
	cv::Mat get_center_header() const;
	cv::Mat get_right_header() const;


	/* Utility methods to detect pressed buttons */
	static bool is_selected(const cv::Vec4b& point);
	static bool is_tab_selected(const cv::Vec4b& point);

private:
	image_recognition& recog;
	cv::Mat prev_islands;
	tab open_tab;

	cv::Mat screenshot;

	std::map<std::string, unsigned int> island_to_session;

	// empty if not yet evaluated, use get_selected_island()
	std::string selected_island;
	// use get_selected_session()
	unsigned int selected_session;
	unsigned int center_pane_selection;

	
	tab compute_open_tab() const;
	void update_islands();


};

}
