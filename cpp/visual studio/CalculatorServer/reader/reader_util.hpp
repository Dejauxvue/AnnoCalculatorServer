#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <vector>
#include <map>
#include <string>

#include <opencv2/opencv.hpp>

#include <tesseract/baseapi.h>

 #define SHOW_CV_DEBUG_IMAGE_VIEW
// #define CONSOLE_DEBUG_OUTPUT

struct keyword_dictionary
{
	std::map<unsigned int, std::string> population_levels;
	std::map<unsigned int, std::string> factories;
	std::map<unsigned int, std::string> products;
	std::map<unsigned int, std::string> ui_texts;
};

class image_recognition;


enum class phrase
{
	ALL_ISLANDS = 22381,
	PRODUCTION = 22365,
	STATISTICS = 22438,
	THE_NEW_WORLD = 180025,
	THE_OLD_WORLD = 180023,
	CAPE_TRELAWNEY = 110934,
	THE_ARCTIC = 180045,
	RESIDENTS = 22379,
	BREAKDOWN = 22434,
	ARCHIBALD_HARBOUR = 100680,
	ANNE_HARBOUR = 100681,
	FORTUNE_HARBOUR = 100682,
	ISABELL_HARBOUR = 100683,
	ELI_HARBOUR = 100685,
	KAHINA_HARBOUR = 100686,
	NATE_HARBOUR = 101117,
	INUIT = 237,
	ARCHIBALD = 240,
	ANNE = 73,
	FORTUNE = 76,
	ISABELL = 29,
	ELI = 46,
	KAHINA = 78,
	NATE = 77,
};

class image_recognition
{

public:
	image_recognition();

	static std::string to_string(const std::wstring&);
	static std::wstring to_wstring(const std::string&);




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
	static std::vector<unsigned int> get_guid_from_name(const cv::Mat& text,
		const std::map<unsigned int, std::string>& dictionary);

	/** 
	* Removes all GUIDs from factories if that factory cannot be build in the specified session
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
	void update(const std::string& language = std::string("english"), 
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

	std::string join(const std::vector<std::pair<std::string, cv::Rect>>& words, bool insert_sapces = false) const;

	/**
	* access to the singleton TessBaseAPI instance
	*/
	//@{
	void update_ocr(const std::string& language);
	static std::shared_ptr<tesseract::TessBaseAPI> ocr;
	//@}



	/*
	* Checks whether there is a dictionary for @param{language}
	*/
	bool has_language(const std::string& language) const;

	/*
	* Returns the dictionary currently in use.
	*/
	const keyword_dictionary& get_dictionary() const;

	cv::Mat get_screenshot() const;

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

	/*
	* Parses the integer contained in @param{word}
	* Replaces letters commonly from wrongly detected digits (e.g. O instead of 0)
	* Discards all other symbols.
	* Returns MIN_INTEGER on failure
	*/
	static int number_from_string(const std::string& word);

	static const std::map<char, char> letter_to_digit;
	static const std::string ALL_ISLANDS;



	cv::Mat screenshot;
	std::string language;



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

