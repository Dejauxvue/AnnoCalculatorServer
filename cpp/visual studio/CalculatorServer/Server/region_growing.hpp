#pragma once

#include <opencv2/opencv.hpp>


struct comparePoints {
	bool operator()(const cv::Point & a, const cv::Point & b) const{
		if (a.x != b.x)
			return a.x < b.x;
		return a.y < b.y;
	}
};

class region_growing
{
public:

	static cv::Rect get_aa_bb(const std::list<cv::Point>&);

	static std::list<cv::Point> find_rgb_region(cv::InputArray in, const cv::Point& seed, float threshold);
	static cv::Mat take_screenshot();
	
};

