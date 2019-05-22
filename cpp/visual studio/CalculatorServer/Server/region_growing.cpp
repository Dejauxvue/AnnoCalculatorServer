#include "region_growing.hpp"

#define NOMINMAX
#include <windows.h>

#include <list>
#include <opencv2/opencv.hpp>



std::list<cv::Point> region_growing::find_rgb_region(cv::InputArray in, const cv::Point& seed, float threshold)
{
	cv::Mat input = in.getMat();

	const cv::Rect img_rect = cv::Rect({ 0,0 }, in.size());

	std::set < cv::Point, comparePoints> open;
	open.insert(seed);
	const auto seed_color = input.at<cv::Vec4b>(seed);

	/*auto cmp = [](cv::Point a, cv::Point b) {
		if (a.x != b.x)
			return a.x < b.x;
		return a.y < b.y;
	};*/

	//std::set<cv::Point, decltype(cmp)> closed(cmp);
	std::set<cv::Point, comparePoints> closed;

	std::list<cv::Point> ret;

	while (!open.empty()) {
		const cv::Point current_point = *open.begin();
		open.erase(open.begin());


		const auto cc = input.at<cv::Vec4b>(current_point);
		const int color_diff = (cc.val[0] - int(seed_color.val[0])) * (cc.val[0] - int(seed_color.val[0]))
			+ (cc.val[1] - int(seed_color.val[1])) * (cc.val[1] - int(seed_color.val[1]))
			+ (cc.val[2] - int(seed_color.val[2])) * (cc.val[2] - int(seed_color.val[2]))
			+ (cc.val[3] - int(seed_color.val[3])) * (cc.val[3] - int(seed_color.val[3]));
		if (color_diff
			< threshold) 
		{
			ret.push_back(current_point);
			cv::Point new_candidates[4] = {
				current_point + cv::Point(1,0),
				current_point + cv::Point(0,1),
				current_point + cv::Point(-1,0),
				current_point + cv::Point(0,-1)
			};
			for (int i = 0; i < 4; i++) {
				if (new_candidates[i].inside(img_rect)
					&& closed.find(new_candidates[i]) == closed.end()) 
				{
					open.insert(new_candidates[i]);
				}
			}
		}
		closed.insert(current_point);
	}

	
	//input[seed];

	return ret;
}

cv::Rect region_growing::get_aa_bb(const std::list<cv::Point>& input)
{
	if (input.empty())
		return cv::Rect(cv::Point(0,0), cv::Point( 0,0 ));

	cv::Point min(*input.begin());
	cv::Point max(*input.begin());

	for (const cv::Point& p : input) {
		min.x = std::min(min.x, p.x);
		min.y = std::min(min.y, p.y);
		max.x = std::max(max.x, p.x);
		max.y = std::max(max.y, p.y);
	}
	return cv::Rect(min , max + cv::Point(1, 1));
}

cv::Mat region_growing::take_screenshot()
{
	cv::Mat src;
	BITMAPINFOHEADER  bi;

	HDC hwindowDC = GetDC(nullptr);
	HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

	RECT windowsize;    // get the height and width of the screen
	GetClientRect(nullptr, &windowsize);

	const int srcheight = 1080;// windowsize.bottom;
	const int srcwidth = 1920;// windowsize.right;
	const int height = 1080;//windowsize.bottom / 1;  //change this to whatever size you want to resize to
	const int width = 1920;//windowsize.right / 1;

	src.create(height, width, CV_8UC4);

	// create a bitmap
	HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
	bi.biSize = sizeof(BITMAPINFOHEADER);    //http://msdn.microsoft.com/en-us/library/windows/window/dd183402%28v=vs.85%29.aspx
	bi.biWidth = width;
	bi.biHeight = -height;  //this is the line that makes it draw upside down or not
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	// use the previously created device context with the bitmap
	SelectObject(hwindowCompatibleDC, hbwindow);
	// copy from the window device context to the bitmap device context
	StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, reinterpret_cast<BITMAPINFO *>(&bi), DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

	// avoid memory leak
	DeleteObject(hbwindow);
	DeleteDC(hwindowCompatibleDC);
	ReleaseDC(NULL, hwindowDC);

	return src;
}
