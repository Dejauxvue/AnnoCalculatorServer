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
	static std::map < std::string, int> get_anno_population_tesserarct_ocr();

private:

	/**
	* load an image in the cv-Format used here
	*/
	static cv::Mat load_image(const std::string&);

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
};

