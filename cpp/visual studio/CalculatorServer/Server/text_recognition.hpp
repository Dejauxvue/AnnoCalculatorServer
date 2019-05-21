#pragma once

#include <vector>
#include <map>
#include <string>
#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>


class text_recognition
{
public:
	static std::vector<std::pair<std::string, cv::Rect>>  detect_words(const cv::InputArray&);

	static std::map<std::string, int> get_anno_population(const std::vector<std::pair<std::string, cv::Rect>>&);

private:
	static std::shared_ptr<tesseract::TessBaseAPI> ocr();
	static std::shared_ptr<tesseract::TessBaseAPI> ocr_;
};

