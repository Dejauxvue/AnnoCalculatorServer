#include "image_recognition.hpp"

#define NOMINMAX
#include <windows.h>

#include <algorithm>
#include <regex>
#include <list>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>

#include <opencv2/opencv.hpp>


//#define SHOW_CV_DEBUG_IMAGE_VIEW

struct comparePoints {
	bool operator()(const cv::Point& a, const cv::Point& b) const {
		if (a.x != b.x)
			return a.x < b.x;
		return a.y < b.y;
	}
};

std::list<cv::Point> image_recognition::find_rgb_region(cv::InputArray in, const cv::Point& seed, float threshold)
{
	cv::Mat input = in.getMat();

	const cv::Rect img_rect = cv::Rect({ 0,0 }, in.size());

	std::set < cv::Point, comparePoints> open;
	open.insert(seed);
	const auto seed_color = input.at<cv::Vec4b>(seed);

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

std::map<std::string, int> image_recognition::get_anno_population()
{

	const auto fit_criterion = [](float e) {return e > 0.8f; };

	struct template_images {
		cv::Mat island_digits[10];
		std::map<std::string, cv::Mat> island_pop_types;
		cv::Mat island_pop_symbol;
	};

	static template_images templates = []() {
		template_images ret;
		for (int i = 0; i < 10; i++) {
			ret.island_digits[i] = load_image("image_recon/island_" + std::to_string(i) + ".bmp");
		}
		std::string popluation_names[] = { "farmers", "workers"/*, "artisans", "engineers", "investors", "jornaleros", "obreros" */ };
		for (const auto& n : popluation_names) {
			ret.island_pop_types.insert({ n, load_image("image_recon/island_" + n + "_icon.bmp") });
		}
		ret.island_pop_symbol = load_image("image_recon/population_symbol_with_bar.png");
		return ret;
	}();


	cv::Mat im = take_screenshot();
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("image_recon/last_screenshot.png", im);
#endif //SHOW_CV_DEBUG_IMAGE_VIEW
	//im = im(cv::Rect(cv::Point(im.cols / 4, 0), cv::Size(im.cols / 2, im.rows / 2)));
	im = im(cv::Rect(cv::Point(0, 0), cv::Size(im.cols, im.rows / 2)));

	const auto pop_symbol_match_result = match_template(im, templates.island_pop_symbol);

	if (!fit_criterion(pop_symbol_match_result.second)) {
		std::cout << "can't find population" << std::endl;
		return std::map<std::string, int>();
	}

	auto region = find_rgb_region(im, pop_symbol_match_result.first.br(), 1);
	cv::Rect aa_bb = get_aa_bb(region);
	if (aa_bb.area() <= 0)
		return std::map<std::string, int>();

	cv::Mat cropped_image = im(aa_bb);
	std::map<std::string, cv::Point> pop_symbol_positions;
	for (const auto& t : templates.island_pop_types) {
		const auto match_result = match_template(cropped_image, t.second);
		if (fit_criterion(match_result.second)) {
			pop_symbol_positions.insert({ t.first, match_result.first.tl() });
		}
	}

	std::vector<std::vector<cv::Point>> digit_positions;
	digit_positions.resize(10);
	for (int i = 0; i < 10; i++) {
		std::cout << "searching " << i << "s" << std::endl;
		const auto match_result = match_all_occurences(cropped_image, templates.island_digits[i], [&](float e) {
			return !fit_criterion(e); });
		digit_positions[i].insert(digit_positions[i].end(), match_result.begin(), match_result.end());
	}

	std::vector<std::pair<int, cv::Point>> digits_and_positions;
	for (int i = 0; i < 10; i++) {
		for (const auto& p : digit_positions[i]) {
			digits_and_positions.push_back({ i, p });
		}
	}

	//begin with logics, no image recognition from here
	std::map<std::string, int> ret;
	for (const auto& t : pop_symbol_positions) {
		int result = 0;
		
		//filter for digits in same row
		std::vector<std::pair<int, cv::Point>> relevant_digits_and_positions;
		relevant_digits_and_positions.resize(digits_and_positions.size());
		auto it = std::copy_if(digits_and_positions.begin(), digits_and_positions.end(), relevant_digits_and_positions.begin(), [&](const auto& pair) {
			return std::abs(pair.second.y - t.second.y) < 4;
			});
		relevant_digits_and_positions.resize(std::distance(relevant_digits_and_positions.begin(), it));

		//sort ascending
		std::sort(relevant_digits_and_positions.begin(), relevant_digits_and_positions.end(), [](const auto& a, auto& b) {
			return a.second.x < b.second.x;
			});

		//from digits to integer number
		for (const auto& d : relevant_digits_and_positions) {
			result = result * 10 + d.first;
		}
		ret.insert({ t.first, result });
	}

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW	
	for (const auto& p : region) {
		im.at<cv::Vec4b>(p) = cv::Vec4b(0, 0, 255, 255);
	}
	cv::rectangle(im, pop_symbol_match_result.first, cv::Scalar(255, 0, 0));
	for (const auto& p : pop_symbol_positions) {
		cv::rectangle(im, cv::Rect(p.second + aa_bb.tl(), cv::Size(20, 20)), cv::Scalar(255, 0, 0));
	}

	for (int i = 0; i < 10; i++) {
		const auto dp = digit_positions[i];
		for (const auto& p : dp) {
			//cv::rectangle(im, cv::Rect(p + aa_bb.tl(), cv::Size(10, 10)), cv::Scalar(255, 0, 0));
			cv::putText(im, std::to_string(i), p + aa_bb.tl(), CV_FONT_BLACK, 0.5, cv::Scalar(255, 255, 255));
		}
	}
	cv::rectangle(im, aa_bb, cv::Scalar(0, 255, 0));
	cv::imwrite("image_recon/last_image.bmp", im);
#endif //SHOW_CV_DEBUG_IMAGE_VIEW

	return ret;
}

cv::Mat image_recognition::load_image(const std::string& path)
{
	cv::Mat img = cv::imread(path, cv::IMREAD_COLOR);
	if (img.size().area() < 1) {
		std::cout << "failed to load " << path << std::endl;
	}
	cvtColor(img, img, CV_BGR2BGRA);
	return img;
}

std::vector<cv::Point> image_recognition::match_all_occurences(cv::InputArray source_img, cv::InputArray template_img, const std::function<bool(float)>& stop_criterion)
{
	std::vector<cv::Point> ret;
	cv::Mat current_image = source_img.getMat().clone();

	while (true) {
#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
		cv::imwrite("image_recon/current_image_match_all.bmp", current_image);
#endif //SHOW_CV_DEBUG_IMAGE_VIEW
		std::pair<cv::Rect, float> current_match = match_template(current_image, template_img);

		if (stop_criterion(current_match.second))
			break;

		ret.push_back(current_match.first.tl());
		cv::rectangle(current_image, current_match.first, cv::Scalar(0, 0, 0), -1);

	}
	return ret;
}

std::pair<cv::Rect, float> image_recognition::match_template(cv::InputArray source, cv::InputArray template_img)
{
	/*cv::Mat input_copy = source.getMat().clone();
	cv::cvtColor(input_copy, input_copy, CV_BGRA2GRAY);
	cv::threshold(input_copy, input_copy, 127, 255, cv::THRESH_BINARY);
	cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));

	cv::Mat skel(input_copy.size(), CV_8UC1, cv::Scalar(0));
	cv::Mat temp(input_copy.size(), CV_8UC1);

	bool done;
	do
	{
		cv::morphologyEx(input_copy, temp, cv::MORPH_OPEN, element);
		cv::bitwise_not(temp, temp);
		cv::bitwise_and(input_copy, temp, temp);
		cv::bitwise_or(skel, temp, skel);
		cv::erode(input_copy, input_copy, element);

		double max;
		cv::minMaxLoc(input_copy, 0, &max);
		done = (max == 0);
	} while (!done);

	cv::imshow("binary", skel);
	cv::waitKey(0);*/

	cv::Mat result;
	cv::matchTemplate(source, template_img, result, CV_TM_CCOEFF_NORMED);
	cv::Point min_loc, max_loc;
	double min, max;
	cv::minMaxLoc(result, &min, &max, &min_loc, &max_loc);

	cv::Point template_position = max_loc;

	std::cout << "min: " << min;
	std::cout << "max: " << max << std::endl;

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("image_recon/match_image.bmp", (result) * 256);
#endif //SHOW_CV_DEBUG_IMAGE_VIEW

	return { cv::Rect(template_position, template_img.size()), max };
}

cv::Rect image_recognition::get_aa_bb(const std::list<cv::Point>& input)
{
	if (input.empty())
		return cv::Rect(cv::Point(0, 0), cv::Point(0, 0));

	cv::Point min(*input.begin());
	cv::Point max(*input.begin());

	for (const cv::Point& p : input) {
		min.x = std::min(min.x, p.x);
		min.y = std::min(min.y, p.y);
		max.x = std::max(max.x, p.x);
		max.y = std::max(max.y, p.y);
	}
	return cv::Rect(min, max + cv::Point(1, 1));
}

cv::Mat image_recognition::take_screenshot()
{
	cv::Mat src;

	//std::string window_name_regex_string(".*Anno.*");
	std::string window_name_regex_string(".*IrfanView.*");
	std::regex window_name_regex(window_name_regex_string.data());

	HWND hwnd = NULL;
	struct lambda_parameter {
		HWND* hwnd_p;
		std::regex window_name_regex;
	} params{ &hwnd, window_name_regex };

	EnumWindows(
		[](HWND local_hwnd, LPARAM lparam) {
			int length = GetWindowTextLength(local_hwnd);
			char* buffer = new char[length + 1];
			GetWindowText(local_hwnd, buffer, length + 1);
			std::string windowTitle(buffer);
			if (IsWindowVisible(local_hwnd) || length == 0) {
				return TRUE;
			}
			if (std::regex_match(windowTitle, ((lambda_parameter*)lparam)->window_name_regex)) {
				*(((lambda_parameter*)lparam)->hwnd_p) = local_hwnd;
				return FALSE;

			}
			return TRUE;
		}, (LPARAM)& params);

	if (hwnd == NULL)
	{
		std::cout << "Can't find window with regex " << window_name_regex_string << std::endl
			<< "open windows are:" << std::endl;

		//print all open window titles
		EnumWindows([](HWND hWnd, LPARAM lparam) {
			int length = GetWindowTextLength(hWnd);
			char* buffer = new char[length + 1];
			GetWindowText(hWnd, buffer, length + 1);
			std::string windowTitle(buffer);
			std::cout << hWnd << ":  " << windowTitle << std::endl;
			return TRUE;
			}, NULL);

		return cv::Mat();
	}

	HDC hwindowDC = GetDC(NULL);
	HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

	RECT windowsize;    // get the height and width of the screen
	GetWindowRect(hwnd, &windowsize);

	const int height = windowsize.bottom - windowsize.top;  //change this to whatever size you want to resize to
	const int width = windowsize.right - windowsize.left;

	src.create(height, width, CV_8UC4);

	// create a bitmap
	HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, windowsize.right - windowsize.left, windowsize.bottom - windowsize.top);

	BITMAPINFOHEADER  bi;
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
	StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, windowsize.left, windowsize.top, width, height, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

	// avoid memory leak
	DeleteObject(hbwindow);
	DeleteDC(hwindowCompatibleDC);
	ReleaseDC(hwnd, hwindowDC);

	return src;
}