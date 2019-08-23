#pragma once

#include <vector>
#include <map>
#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/opencv.hpp>

#include <tesseract/baseapi.h>

class image_recognition
{
public:
	/**
	* main routine
	* statically initializes templates to match and ocr api
	*
	* returns a map with entries for all detected population types referred by their english name
	*/
	static std::map < std::string, int> get_anno_population_tesserarct_ocr(const cv::Mat& im);

	/**
	* load an image in the cv-Format used here
	*/
	static cv::Mat load_image(const std::string&);

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

	/**
	* find the best position of [template_img] within [source]
	*
	* returns rectangle of the best template position and the fitting error
	*/
	static std::pair<cv::Rect, float> match_template(cv::InputArray source, cv::InputArray template_img);

	/**
	* make a screenshot from the Anno 7.exe application
	*/
	static cv::Mat take_screenshot();

	/**
	* detect arbitrary words in the given image [in]
	*
	* return a vector of pairs of detected words and their respective bounding box
	*/
	static std::vector<std::pair<std::string, cv::Rect>>  detect_words(
		const cv::InputArray& in);

	/**
	* access to the singleton TessBaseAPI instance
	*/
	//@{
	static std::shared_ptr<tesseract::TessBaseAPI> ocr();
	static std::shared_ptr<tesseract::TessBaseAPI> ocr_;
	//@}

	/**
	* from the detected words [ocr_result] with thei bounding boxes (unused)
	* 
	* returna map with entries for all detected population types referred by their english name
	*/
	static std::map<std::string, int> get_anno_population_from_ocr_result(
		const std::vector<std::pair<std::string, cv::Rect>>& ocr_result);

	/**
	* compute the similiarity of two strings using Jaro Similarity
	* source: https://codereview.stackexchange.com/questions/187969/c-edit-distance-string-similarity-function-based-on-the-jaro-winkler-algorit
	*
	* results are 1 for matching strings and 0 for very different strings
	*/
	static float jaro_winkler_similarity(const std::string& source,
		const std::string& target,
		const std::size_t prefix = 2,
		const float boost_treshold = 0.7,
		const float scaling_factor = 0.1);

	/**
	* compute the similiarity of two strings using Jaro Similarity
	* source: https://codereview.stackexchange.com/questions/187969/c-edit-distance-string-similarity-function-based-on-the-jaro-winkler-algorit
	*/
	static float jaro_similarity(const std::string& source, const std::string& target);
	
};

