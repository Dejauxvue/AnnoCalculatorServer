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

	static std::map<std::string, int> get_anno_population();

private:
	static cv::Rect get_aa_bb(const std::list<cv::Point>&);

	static std::list<cv::Point> find_rgb_region(cv::InputArray in, const cv::Point& seed, float threshold);
	static cv::Mat take_screenshot();

	static std::vector<std::pair<std::string, cv::Rect>>  detect_words(const cv::InputArray&);

	static std::map<std::string, int> get_anno_population_from_ocr_result(const std::vector<std::pair<std::string, cv::Rect>>&);

	static std::shared_ptr<tesseract::TessBaseAPI> ocr();
	static std::shared_ptr<tesseract::TessBaseAPI> ocr_;
	
};

