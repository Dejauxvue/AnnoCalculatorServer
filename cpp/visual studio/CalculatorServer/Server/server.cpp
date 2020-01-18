#include "server.hpp"
#include "image_recognition.hpp"

using namespace std;
using namespace web;
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;

/* IMPORTANT!!: this should be updated to the current release tag*/
const std::string server::VERSION_TAG("v2.2.2");

server::server(utility::string_t url) : m_listener(url)
{
	m_listener.support(methods::GET, std::bind(&server::handle_get, this, std::placeholders::_1));
}


void server::read_anno_population(web::json::value& result)
{

	auto population = image_recog.get_population_amount();

	for (const auto& p : population) {
		web::json::value entry;
		if (result.has_field(std::to_wstring(p.first)))
			entry = result.at(std::to_wstring(p.first));

		entry[std::wstring(L"amount")] = web::json::value(p.second);
		result[std::to_wstring(p.first)] = entry;
	}
}

void server::read_buildings_count(web::json::value& result)
{


	auto values = image_recog.get_assets_existing_buildings();

	for (const auto& p : values) {
		web::json::value entry;
		if (result.has_field(std::to_wstring(p.first)))
			entry = result.at(std::to_wstring(p.first));

		entry[std::wstring(L"existingBuildings")] = web::json::value(p.second);
		result[std::to_wstring(p.first)] = entry;
	}

}

void server::read_productivity_statistics(web::json::value& result, bool optimal_productivity)
{
	auto values = optimal_productivity ? image_recog.get_optimal_productivities() : image_recog.get_average_productivities();

	for (const auto& p : values) {
		web::json::value entry;
		if (result.has_field(std::to_wstring(p.first)))
			entry = result.at(std::to_wstring(p.first));

		entry[std::wstring(L"percentBoost")] = web::json::value(p.second);
		result[std::to_wstring(p.first)] = entry;
	}
}

void server::handle_get(http_request request)
{
	if (mutex_.try_lock()) {
		const auto& query_params = request.absolute_uri().split_query(request.absolute_uri().query());

		ucout << "request received: " << request.absolute_uri().query() << endl;

//		auto res = image_recog.find_icon(image_recognition::take_screenshot(), image_recognition::load_image("image_recon/icons/icon_resident.png"), cv::Scalar(57, 57, 57, 255));
		//cv::Mat roi = image_recognition::take_screenshot()({ 670,670 + 160 } , { 1016,1016 + 50 });
		//cv::imwrite("image_recon/roi.png", roi);
		//auto words = image_recog.detect_words(roi, "english");
//		image_recog.get_productivity(image_recognition::take_screenshot(), "english");
		


		std::string language("english");
		bool optimal_productivity = false;
		if (!query_params.empty())
		{
			if (query_params.find(L"lang") != query_params.end())
			{
				std::string lang = image_recog.to_string(query_params.find(L"lang")->second);
				if (image_recog.has_language(lang))
					language = lang;
			}
			
			if (query_params.find(L"optimalProductivity") != query_params.end())
			{
				std::wstring string = query_params.find(L"optimalProductivity")->second;
				optimal_productivity = string.compare(L"true") == 0 || string.compare(L"1") == 0;
			}

		}

		cv::Mat screenshot(image_recog.update(language));
		if (screenshot.empty())
		{
			std::cout << "Couldn't take screenshot" << std::endl;
			web::http::http_response response(status_codes::NoContent);
			response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
			const auto t = request.reply(response);
			mutex_.unlock();
			return;
		}

		web::json::value json_message;
		read_anno_population(json_message);
		read_buildings_count(json_message);
		read_productivity_statistics(json_message, optimal_productivity);

		json_message[U("version")] = web::json::value(std::wstring(VERSION_TAG.begin(), VERSION_TAG.end()));
		json_message[U("islandName")] = web::json::value(image_recognition::to_wstring(image_recog.get_selected_island()));

		web::http::http_response response(status_codes::OK);
		response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
		response.set_body(json_message);
		const auto t = request.reply(response);
		mutex_.unlock();
	}
	else {
		web::http::http_response response(status_codes::TooManyRequests);
		response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
		const auto t = request.reply(response);
	}
};