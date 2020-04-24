#include "server.hpp"

#include "version.hpp"

using namespace std;
using namespace web;
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;

using namespace reader;



server::server(bool verbose)
	:
	recog(verbose),
	stats(recog)
{
}

server::server(bool verbose, utility::string_t url) :
	recog(verbose),
	stats(recog),
	m_listener(url)
{
	m_listener.support(methods::GET, std::bind(&server::handle_get, this, std::placeholders::_1));
}


void server::read_anno_population(web::json::value& result)
{

	auto population = stats.get_population_amount();

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


	auto values = stats.get_assets_existing_buildings();

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
	auto values = optimal_productivity ? stats.get_optimal_productivities() : stats.get_average_productivities();

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


		std::string language("english");
		bool optimal_productivity = false;
		if (!query_params.empty())
		{
			if (query_params.find(L"lang") != query_params.end())
			{
				std::string lang = image_recognition::to_string(query_params.find(L"lang")->second);
				if (stats.has_language(lang))
					language = lang;
			}
			
			if (query_params.find(L"optimalProductivity") != query_params.end())
			{
				std::wstring string = query_params.find(L"optimalProductivity")->second;
				optimal_productivity = string.compare(L"true") == 0 || string.compare(L"1") == 0;
			}

		}



		cv::Rect2i window(recog.find_anno());
		if (!window.area())
		{
			std::cout << "Couldn't take screenshot" << std::endl;
			web::http::http_response response(status_codes::NoContent);
			response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
			const auto t = request.reply(response);
			mutex_.unlock();
			return;
		}

		cv::Mat screenshot(recog.take_screenshot(window));
		stats.update(language, screenshot);

		web::json::value json_message;
		read_anno_population(json_message);
		read_buildings_count(json_message);
		read_productivity_statistics(json_message, optimal_productivity);

		json_message[U("version")] = web::json::value(std::wstring(version::VERSION_TAG.begin(), version::VERSION_TAG.end()));
		json_message[U("islandName")] = web::json::value(image_recognition::to_wstring(stats.get_selected_island()));

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