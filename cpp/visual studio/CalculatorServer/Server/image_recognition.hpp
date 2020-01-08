#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <vector>
#include <map>
#include <string>

#include <opencv2/opencv.hpp>

#include <tesseract/baseapi.h>


struct keyword_dictionary
{
	std::map<unsigned int, std::string> population_levels;
	std::map<unsigned int, std::string> factories;
	std::map<unsigned int, std::string> products;
	std::map<unsigned int, std::string> ui_texts;
};

class image_recognition;

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
		STORAGE = 2
	};

	std::map<std::string, unsigned int> island_to_session;

	statistics_screen(image_recognition& recog);

	void update(const cv::Mat& screenshot);

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
	static bool closer_to(const cv::Scalar& color, const cv::Scalar& ref, const cv::Scalar& other);

	/* extract region from image, coordinates from [0,1]² */
	static cv::Mat get_square_region(const cv::Mat& img, const cv::Rect2f& rect);
	static cv::Mat get_cell(const cv::Mat& img, float crop_left, float width, float crop_vertical = 0.1f);

	static const std::string all_islands;
	static const cv::Scalar background_brown_light;
	static const cv::Scalar background_blue_dark;
	static const cv::Scalar foreground_brown_light;
	static const cv::Scalar foreground_brown_dark;
	static const cv::Scalar expansion_arrow;

	static const cv::Rect2f position_factory_icon;
	static const cv::Rect2f position_small_factory_icon;
	static const cv::Rect2f position_population_icon;
	

private:
	image_recognition& recog;
	cv::Mat screenshot;
	cv::Mat prev_islands;
	tab open_tab;

	cv::Mat get_pane(const cv::Rect2f& rect) const;
	tab compute_open_tab() const;
	void update_islands();

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

enum class phrase
{
	ALL_ISLANDS = 22381,
	PRODUCTIVITY = 22438,
	PRODUCTION = 22365,
	STATISTICS = 22438,
	THE_NEW_WORLD = 180025,
	THE_OLD_WORLD = 180023,
	CAPE_TRELAWNEY = 110934,
	THE_ARCTIC = 180045,
	RESIDENTS = 22379,
	BREAKDOWN = 22434
};

class image_recognition
{

public:
	image_recognition();

	static std::string to_string(const std::wstring&);
	static std::wstring to_wstring(const std::string&);

	/**
	*
	* returns a map with entries for all detected population types referred by their GUID
	*/
	std::map < unsigned int, int> get_population_amount();
	std::map < unsigned int, int> get_population_amount_from_hud();
	std::map < unsigned int, int> get_population_amount_from_statistic_screen() const;



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
	std::map < unsigned int, int> get_factories_existing_buildings_from_production_screen();
	std::map<unsigned int, int> get_population_existing_buildings_from_statistic_screen() const;

	std::map<unsigned int, int> get_population_workforce_from_statistic_screen() const;

	/*
	* Returns the name of the selected island in the left pane of
	* the statistics screen
	* Returns ALL_ISLANDS
	*/
	std::string get_selected_island();
	unsigned int get_selected_session();
	static const std::string ALL_ISLANDS;

	/*
	* Returns all islands seen in the statistics screen so far
	* Returns island name and associated session
	*/
	std::map<std::string, unsigned int> get_islands() const;

	

	/**
	* load an image in the cv-Format used here
	*/
	static cv::Mat load_image(const std::string&);

	/**
	* creates BGRA image with only black and white pixels
	* thresholding is between two peeks of input image
	*/
	static cv::Mat binarize(cv::InputArray input, bool invert = false);

	/**
	* creates BGRA image with only black and white pixels
	* uses edge detection to get the largest closed monochrome spot
	* fills the spot with white and crops margins
	*
	* @attend returned image might have smaller resolution
	*/
	static cv::Mat binarize_icon(cv::InputArray input, cv::Size target_size = cv::Size());

	/**
	* create the two channel image with H from HLS space
	* from a given BGR image
	*/
	static cv::Mat convert_color_space_for_template_matching(cv::InputArray bgr_in);

	/**
	* create an image with one channel of gamma invariant hue from given BGR image
	* method used is from: https://pdfs.semanticscholar.org/6c16/b450648a531c3ce47db7db3a7794e2f55d96.pdf
	* "Hue that is invariant to brightness and gamma" by Graham Finlayson and Gerald Schaefer, 2001
	*/
	static cv::Mat gamma_invariant_hue_finlayson(cv::InputArray bgr_in);

	/**
	* stores the single channels of an image
	* i-th channel is stored as path_i_.png
	*/
	static void write_image_per_channel(const std::string& path, cv::InputArray img);

	/**
	* the axis-aligned-bounding box from the given set of points
	*/
	static cv::Rect get_aa_bb(const std::list<cv::Point>&);

	/**
	* perform region growing on image [in] 
	* starting at position [seed]
	* expand to all neighbouring pixels with squared color distance to seed < [threshold]
	*
	* return pixel positions of the final region
	*/
	static std::list<cv::Point> find_rgb_region(cv::InputArray in, 
		const cv::Point& seed, float threshold);

	/*
	* Searches for population icon of of tooltip of HUD
	*/
	cv::Rect find_population_icon();

	/*
	* Prints the image icon with alpha channel on top of an equal sized image filled with background_color
	*/
	static cv::Mat blend_icon(cv::InputArray icon, cv::Scalar background_color);

	/*
	* Sets the rgb channels of icon to color, the alpha channel remains untouched.
	*/
	static cv::Mat dye_icon(cv::InputArray icon, cv::Scalar color);

	/**
	* Finds all occurences of the passed icon on top of some region with background_color.
	* Note: icon must precisely have the same size as the pattern to be found in source.
	*/
	static std::pair<cv::Rect, float> find_icon(cv::InputArray source, cv::InputArray icon, cv::Scalar background_color);

	/*
	* Compares icon to the set of icons from the dictionary and returns the best matches or an empty vector in case of failure
	* The input icon must be cropped to the precisely boundaries of the icon. Even small will result in undefined results.
	* The input icon must be a square.
	*/
	std::vector<unsigned int> get_guid_from_icon(cv::Mat icon,
		const std::map<unsigned int, cv::Mat>& dictionary) const;

	/*
	* Returns the session id or 0 in case of failure.
	* Expects a (basically) two colored image, the icon can be somewhere within the image
	*/
	unsigned int get_session_guid(cv::Mat icon) const;

	/*
	* Performs text recognition on the input image.
	* Returns the GUIDs of the best matching text or an empty vector in case of failure
	* The contained text must be on a single line, black with white background and should contain
	* as less noise as possible
	*/
	std::vector<unsigned int> get_guid_from_name(const cv::Mat& text,
		const std::map<unsigned int, std::string>& dictionary) const;

	/** 
	* Removes all GUIDs from factories if that factorie cannot be build in the specified session
	*/
	void filter_factories(std::vector<unsigned int>& factories, unsigned int session) const;

	/*
	* Compares hu moments.
	*/
	static double compare_hu_moments(const std::vector<double>& ma, const std::vector<double>& mb) ;

	/**
	* find the best position of [template_img] within [source]
	*
	* returns rectangle of the best template position and the fitting error
	*/
	static std::pair<cv::Rect, float> match_template(cv::InputArray source, cv::InputArray template_img);

	/**
	* makes a screenshot from the Anno 7.exe application if no image is provided
	* successive calls to getters will evaluate this screenshot
	*/
	cv::Mat update(const std::string& language = std::string("english"), 
		const cv::Mat& img = cv::Mat());
	static cv::Mat take_screenshot();

	/**
	* detect arbitrary words in the given image [in]
	*
	* return a vector of pairs of detected words and their respective bounding box
	*/
	static std::vector<std::pair<std::string, cv::Rect>>  detect_words(
		const cv::InputArray& in,
		tesseract::PageSegMode mode = tesseract::PSM_SPARSE_TEXT);

	/**
	* Returns the length of the longest common subsequence of X and Y
	*/
	static int lcs_length(std::string X, std::string Y);

	/**
	* access to the singleton TessBaseAPI instance
	*/
	//@{
	void update_ocr(const std::string& language);
	static std::shared_ptr<tesseract::TessBaseAPI> ocr;
	//@}

	/**
	* from the detected words [ocr_result] with thei bounding boxes (unused)
	* 
	* returna map with entries for all detected population types referred by their english name
	*/
	std::map<unsigned int, int> get_anno_population_from_ocr_result(
		const std::vector<std::pair<std::string, cv::Rect>>& ocr_result) const;

	/*
	* Checks whether there is a dictionary for @param{language}
	*/
	bool has_language(const std::string& language) const;

	/*
	* Returns the dictionary currently in use.
	*/
	const keyword_dictionary& get_dictionary() const;


	/*
	* Compose custom dictionary from phrases
	*/
	std::map<unsigned int, std::string> make_dictionary(const std::vector<phrase>& list) const;

	/*
	* Computes hu moments of the image.
	*/
	static std::vector<double> get_hu_moments(cv::Mat img);

	static cv::Mat detect_edges(const cv::Mat& im);

	/*
	* Detects contours in the image and filters width wide horizontal lines
	*/
	static std::vector<int> find_horizontal_lines(const cv::Mat& im);

	/*
	* Iterates the rows of a table specified by horizontal lines
	*/
	static void iterate_rows(const cv::Mat& im,
										const std::function<void(const cv::Mat& row)> f);



	/*
	* Returns an integer contained in im.
	* Returns MIN_INTEGER on failure
	*/
	int number_from_region(const cv::Mat& im) const;

private:
	statistics_screen stats_screen;


	cv::Mat screenshot;
	std::string language;
	// empty if not yet evaluated, use get_selected_island()
	std::string selected_island;
	// use get_selected_session()
	unsigned int selected_session;
	unsigned int center_pane_selection;
	// -1 if not searched for, 0 if not found
	cv::Rect population_icon_position;

	std::map<std::string, keyword_dictionary> dictionaries;
	std::map<unsigned int, cv::Mat> product_icons;
	std::map<unsigned int, cv::Mat> factory_icons;
	std::map<unsigned int, cv::Mat> population_icons;
	std::map<unsigned int, cv::Mat> session_icons;
	std::map<unsigned int, unsigned int> factory_to_region;
	std::map<unsigned int, unsigned int> session_to_region;
	static const unsigned int REGION_META = 5000005;
	static const unsigned int SESSION_META = 180039;
	std::map<unsigned int, std::vector<unsigned int>> product_to_factories;

	static const std::map<std::string, std::string> tesseract_languages;
};

