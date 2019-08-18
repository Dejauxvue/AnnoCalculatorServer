#include "image_recognition.hpp"

#define NOMINMAX
#include <windows.h>

#include <algorithm>
#include <regex>
#include <list>
#include <filesystem>
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
			<= threshold)
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
	return ret;
}

std::pair<cv::Rect, float> image_recognition::match_template(cv::InputArray source, cv::InputArray template_img)
{
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

std::map<std::string, int> image_recognition::get_anno_population_tesserarct_ocr( const cv::Mat& im)
{
	const auto fit_criterion = [](float e) {return e > 0.9f; };

	struct template_images {
		cv::Mat island_pop_symbol;
	};

	static template_images templates = [&]() {
		std::string resolution_string = std::to_string(im.cols) + "x" + std::to_string(im.rows);
		std::cout << "detected resolution: " << resolution_string
			<< ". If this does not match your game's resolution or the resolution changes, restart this application!"
			<< std::endl;
		template_images ret;
		ret.island_pop_symbol = load_image("image_recon/" + resolution_string + "/population_symbol_with_bar.bmp");
		return ret;
	}();

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("image_recon/last_screenshot.png", im);
#endif //SHOW_CV_DEBUG_IMAGE_VIEW

	cv::Mat im_copy = im(cv::Rect(cv::Point(0, 0), cv::Size(im.cols, im.rows / 2)));

	const auto pop_symbol_match_result = match_template(im_copy, templates.island_pop_symbol);

	if (!fit_criterion(pop_symbol_match_result.second)) {
		std::cout << "can't find population" << std::endl;
		return std::map<std::string, int>();
	}

	auto region = find_rgb_region(im_copy, pop_symbol_match_result.first.br(), 0);
	cv::Rect aa_bb = get_aa_bb(region);
	if (aa_bb.area() <= 0)
		return std::map<std::string, int>();

	cv::Mat cropped_image = im_copy(aa_bb);

	std::vector<cv::Mat> channels(4);
	cv::split(cropped_image, channels);
	channels[0] = 0;
	channels[1] = 255 - channels[1];
	channels[2] = 255 - channels[2];
	channels[3] = 255;
	cv::merge(channels, cropped_image);

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	cv::imwrite("image_recon/last_ocr_input.png", im_copy(aa_bb));
#endif //SHOW_CV_DEBUG_IMAGE_VIEW

	std::vector<std::pair<std::string, cv::Rect>> ocr_result;
	try {
		ocr_result = detect_words(cropped_image);
	}
	catch (const std::exception& e) {
		std::cout << "there was an exception: " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "unknown exception" << std::endl;
	}

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW	
	for (const auto& p : region) {
		im_copy.at<cv::Vec4b>(p) = cv::Vec4b(0, 0, 255, 255);
	}
	cv::rectangle(im_copy, aa_bb, cv::Scalar(0, 255, 0));
	cv::imwrite("image_recon/last_image.bmp", im_copy);
#endif //SHOW_CV_DEBUG_IMAGE_VIEW
	return get_anno_population_from_ocr_result(ocr_result);
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

	std::string window_name_regex_string(".*Anno 7.*");
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
			if (length == 0) {
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
			if (length > 0)
			{
				std::cout << "match result "
					<< std::regex_match(windowTitle, ((lambda_parameter*)lparam)->window_name_regex) << std::endl;
			}
			return TRUE;
			}, (LPARAM)& params);

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

std::shared_ptr<tesseract::TessBaseAPI> image_recognition::ocr_(nullptr);

std::vector<std::pair<std::string, cv::Rect>> image_recognition::detect_words(const cv::InputArray& in)
{
	cv::Mat input = in.getMat();


	ocr()->SetPageSegMode(tesseract::PSM_SPARSE_TEXT);

	// Set image data
	ocr()->SetImage(input.data, input.cols, input.rows, 4, input.step);

	ocr()->Recognize(0);
	tesseract::ResultIterator* ri = ocr()->GetIterator();
	tesseract::PageIteratorLevel level = tesseract::RIL_WORD;

	std::vector<std::pair<std::string, cv::Rect>> ret;

	if (ri != 0) {
		ret.reserve(10);
		do {
			const char* word = ri->GetUTF8Text(level);
			float conf = ri->Confidence(level);
			int x1, y1, x2, y2;
			ri->BoundingBox(level, &x1, &y1, &x2, &y2);
			printf("word: '%s';  \tconf: %.2f; BoundingBox: %d,%d,%d,%d;\n",
				word, conf, x1, y1, x2, y2);
			std::string word_s = word ? std::string(word) : std::string();
			cv::Rect aa_bb(cv::Point(x1, y1), cv::Point(x2, y2));
			ret.push_back(std::make_pair(word_s, aa_bb));
			delete[] word;
		} while (ri->Next(level));
	}

	return ret;
}

std::map<std::string, int> image_recognition::get_anno_population_from_ocr_result(const std::vector<std::pair<std::string, cv::Rect>>& ocr_result)
{
	std::pair<std::string, std::regex> keywords[] = {
		std::make_pair("farmers", std::regex(".*Far?mers.*|.*Baue(rn|m).*")),
		std::make_pair("workers", std::regex(".*Workers.*|.*Arbeiter.*")),
		std::make_pair("artisans", std::regex(".*Artisans.*|.*Handwerker.*")),
		std::make_pair("engineers", std::regex(".*Engineers.*|.*Ingenieure.*")),
		std::make_pair("investors", std::regex(".*Investors.*|.*Investoren.*")),
		std::make_pair("jornaleros", std::regex(".*aleros.*")),
		std::make_pair("obreros", std::regex(".*Obreros.*")) };

	std::map<std::string, int> ret;

	for (const auto& kw : keywords) {
		int population = 0;
		for (const auto& pop_name_word : ocr_result) {
			if (population > 0)
				break;
			if (std::regex_match(pop_name_word.first, kw.second)) {	
				for (const auto& pop_value_word : ocr_result) {
					
					if (pop_name_word == pop_value_word)
						continue;
					if (std::abs(pop_value_word.second.tl().y - pop_name_word.second.tl().y) < 4
						&& pop_value_word.second.tl().x > pop_name_word.second.tl().x) {
						std::string number_string = std::regex_replace(pop_value_word.first, std::regex("\\,|\\.|\\/"), "");
						population = 
							[&]() {
								try { return std::stoi(number_string); }
								catch (...) { std::cout << "could not match number string: " << number_string << std::endl; return 0; }
							}();
						if (population > 0) {	
							break;
						}				
					}
				}	
				//if word based ocr fails, try symbols (probably only 1 digit population)
				if (population == 0) {
					std::cout << "could not find population number, if the number is only 1 digit, this is a known problem" << std::endl;
				}
			}
			
			if (population > 0) {
				auto insert_result = ret.insert(std::make_pair(kw.first, population));
				if (!insert_result.second)
					std::cout << "collision with population names" << std::endl;
				std::cout << "new value for " << kw.first << ": " << population << std::endl;
			}
		}		
	}
	std::cout << "final map: " << std::endl;
	std::for_each(ret.begin(), ret.end(), [](const auto& entry) { std::cout << entry.first << ": " << entry.second << std::endl; });
	return ret;
}

std::shared_ptr<tesseract::TessBaseAPI> image_recognition::ocr()
{
	if (!ocr_) {
		ocr_.reset(new tesseract::TessBaseAPI());
		if (ocr_->Init(NULL, "eng"))
		{
			std::cout << "error initialising tesseract" << std::endl;
		}
		ocr_->SetVariable("CONFIGFILE", "bazaar");
	}
	return ocr_;
}
