#pragma once

#include <vector>
#include <map>
#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/opencv.hpp>


class image_recognition
{
public:

	static std::map<std::string, int> get_anno_population();

private:

	static cv::Mat load_image(const std::string&);

	static std::vector<cv::Point> match_all_occurences(cv::InputArray source_img, 
		cv::InputArray template_img, 
		const std::function<bool(float)>& stop_criterion, 
		cv::Mat* img_erased_digits = nullptr);

	static std::pair<cv::Rect, float> match_template(cv::InputArray source, cv::InputArray template_img);

	static cv::Rect get_aa_bb(const std::list<cv::Point>&);

	static std::list<cv::Point> find_rgb_region(cv::InputArray in, const cv::Point& seed, float threshold);
	static cv::Mat take_screenshot();

	static cv::Mat make_binary(cv::InputArray in);
};

