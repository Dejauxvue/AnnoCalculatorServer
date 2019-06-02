#include "server.hpp"

#include <opencv2/opencv.hpp>

#include "region_growing.hpp"
#include "text_recognition.hpp"


//#define SHOW_CV_DEBUG_IMAGE_VIEW

using namespace std;
using namespace web;
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;

server::server(utility::string_t url) : m_listener(url)
{
	m_listener.support(methods::GET, std::bind(&server::handle_get, this, std::placeholders::_1));

}

web::json::value server::read_anno_population() const
{
	static cv::Mat tmpl = []() {
		std::cout << "Loading template..." << std::endl;
		cv::Mat r = cv::imread("image_recon/population_symbol_with_bar.png", cv::IMREAD_COLOR);
		cvtColor(r, r, CV_BGR2BGRA);
		std::cout << "successfully loaded template" << std::endl;
		return r; 
	}();

	cv::Mat im = region_growing::take_screenshot();
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
		return web::json::value();
	}

	auto region = region_growing::find_rgb_region(im, template_position + cv::Point(tmpl.size()), 1);

	for (const auto& p : region) {
		im.at<cv::Vec4b>(p) = cv::Vec4b(0, 0, 255, 255);
	}

	cv::Rect aa_bb = region_growing::get_aa_bb(region);

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
			cv::imwrite("../../../../image_recon/last_image.png", im);
			ocr_result = text_recognition::detect_words(cropped_image);
		}
		catch (const std::exception& e) {
			std::cout << "there was an exception: " << e.what() << std::endl;
		}
		catch (...)
		{
			std::cout << "unknown exception" << std::endl;
		}
	}

	auto final_result = text_recognition::get_anno_population(ocr_result);

	web::json::value ret;
	for (const auto& p : final_result) {
		ret[std::wstring(p.first.begin(), p.first.end())] = web::json::value::number(p.second);
	}

#ifdef SHOW_CV_DEBUG_IMAGE_VIEW
	///cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);// Create a window for display.
	

	for (const auto& w : ocr_result) {
		cv::rectangle(im, w.second + aa_bb.tl(), cv::Scalar(255, 255, 255));
	}

	cv::rectangle(im, cv::Rect(template_position, tmpl.size()), cv::Scalar(255, 0, 0));
	cv::rectangle(im, aa_bb, cv::Scalar(0, 255, 0));
	cv::imwrite("../../../../image_recon/last_image.png", im);
	//imshow("Display window", im);
	//cv::waitKey(0);
#endif //SHOW_CV_DEBUG_IMAGE_VIEW

	return ret;
}

void server::handle_get(http_request request)
{
	ucout << request.to_string() << endl;

	web::json::value json_message = read_anno_population();

	web::http::http_response response(status_codes::OK);
	response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
	response.set_body(json_message);
	request.reply(response);
};