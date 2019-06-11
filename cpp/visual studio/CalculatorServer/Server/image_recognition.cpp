#include "image_recognition.hpp"

#define NOMINMAX
#include <windows.h>

#include <algorithm>
#include <regex>
#include <list>

#include <opencv2/opencv.hpp>


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
	static cv::Mat tmpl = []() {
		std::cout << "Loading template..." << std::endl;
		cv::Mat r = cv::imread("image_recon/population_symbol_with_bar.png", cv::IMREAD_COLOR);
		cvtColor(r, r, CV_BGR2BGRA);
		std::cout << "successfully loaded template" << std::endl;
		return r;
	}();

	cv::Mat im = take_screenshot();
	cv::imwrite("image_recon/last_screenshot.png", im);
	im = im(cv::Rect(cv::Point(im.cols / 4, 0), cv::Size(im.cols / 2, im.rows / 2)));


	cv::Mat result;
	cv::matchTemplate(im, tmpl, result, CV_TM_SQDIFF);
	cv::Point template_position, max_loc;
	double min, max;
	cv::minMaxLoc(result, &min, &max, &template_position, &max_loc);

	std::cout << "min: " << min;
	std::cout << "max: " << max << std::endl;
	if (min > 73144) {
		std::cout << "can't find population" << std::endl;
		return std::map<std::string, int>();
	}

	auto region = find_rgb_region(im, template_position + cv::Point(tmpl.size()), 1);

	for (const auto& p : region) {
		im.at<cv::Vec4b>(p) = cv::Vec4b(0, 0, 255, 255);
	}

	cv::Rect aa_bb = get_aa_bb(region);

	std::cout << aa_bb << std::endl;
	std::vector<std::pair<std::string, cv::Rect>> ocr_result;
	if (aa_bb.area() > 0) {
		cv::Mat cropped_image = 255 - im(aa_bb);

		try {
			for (const auto& w : ocr_result) {
				cv::rectangle(im, w.second + aa_bb.tl(), cv::Scalar(255, 255, 255));
			}

			cv::rectangle(im, cv::Rect(template_position, tmpl.size()), cv::Scalar(255, 0, 0));
			cv::rectangle(im, aa_bb, cv::Scalar(0, 255, 0));
			cv::imwrite("image_recon/last_image.png", im);
			ocr_result = detect_words(cropped_image);
		}
		catch (const std::exception& e) {
			std::cout << "there was an exception: " << e.what() << std::endl;
		}
		catch (...)
		{
			std::cout << "unknown exception" << std::endl;
		}
	}

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	///cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);// Create a window for display.


	for (const auto& w : ocr_result) {
		cv::rectangle(im, w.second + aa_bb.tl(), cv::Scalar(255, 255, 255));
	}

	cv::rectangle(im, cv::Rect(template_position, tmpl.size()), cv::Scalar(255, 0, 0));
	cv::rectangle(im, aa_bb, cv::Scalar(0, 255, 0));
	cv::imwrite("image_recon/last_image.png", im);
	//imshow("Display window", im);
	//cv::waitKey(0);
#endif //SHOW_CV_DEBUG_IMAGE_VIEW

	return get_anno_population_from_ocr_result(ocr_result);
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


	//HWND active_window = GetForegroundWindow();
	HWND hwnd = FindWindow(TEXT("Anno 7"), NULL);

	if (hwnd == NULL)
	{
		std::cout << "it can't find any 'note' window" << std::endl;
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

	ocr()->SetPageSegMode(tesseract::PSM_AUTO);

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
		std::make_pair("Farmers", std::regex(".*Far?mers.*")),
		std::make_pair("Workers", std::regex(".*Workers.*")),
		std::make_pair("Artisans", std::regex(".*Artisans.*")),
		std::make_pair("Engineers", std::regex(".*Engineers.*")),
		std::make_pair("Investors", std::regex(".*Investors.*")),
		std::make_pair("Jornaleros", std::regex(".*aleros.*")),
		std::make_pair("Obreros", std::regex(".*Obreros.*")) };

	std::map<std::string, int> ret;

	for (const auto& kw : keywords) {
		for (const auto& pop_name_word : ocr_result) {
			if (std::regex_match(pop_name_word.first, kw.second)) {
				for (const auto& pop_value_word : ocr_result) {
					if (pop_name_word == pop_value_word)
						continue;
					if (std::abs(pop_value_word.second.tl().y - pop_name_word.second.tl().y) < 2
						&& pop_value_word.second.tl().x > pop_name_word.second.tl().x) {
						std::string number_string = std::regex_replace(pop_value_word.first, std::regex("\\,|\\."), "");
						int population = std::stoi(number_string);
						if (population > 0) {
							auto insert_result = ret.insert(std::make_pair(kw.first, population));
							std::cout << "new value for " << kw.first << ": " << population << std::endl;
							if (!insert_result.second)
								std::cout << "collision! value already existed: " << insert_result.first->second << std::endl;
						}

					}
				}
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
		//auto     numOfConfigs = 1;
		//auto     **configs = new char *[numOfConfigs];
		//configs[0] = (char *) "E:/libs/Tesseract-OCR/tessdata/configs/bazaar";
		if (ocr_->Init(NULL, "eng"))// , configs, numOfConfigs, nullptr, nullptr, false))
		{
			std::cout << "error initialising tesseract" << std::endl;
		}
		ocr_->SetVariable("CONFIGFILE", "bazaar");
	}
	return ocr_;
}