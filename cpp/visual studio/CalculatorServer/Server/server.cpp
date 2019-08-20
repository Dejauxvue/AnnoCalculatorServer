#include "server.hpp"
#include "image_recognition.hpp"

using namespace std;
using namespace web;
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;

/* IMPORTANT!!: this should be updated to the current release tag*/
const std::string server::VERSION_TAG("v1.0.3");

server::server(utility::string_t url) : m_listener(url)
{
	m_listener.support(methods::GET, std::bind(&server::handle_get, this, std::placeholders::_1));
}


void server::handle_get(http_request request)
{
	ucout << request.to_string() << endl;

	//auto population = image_recognition::get_anno_population_template_matching();
	auto population = image_recognition::get_anno_population_tesserarct_ocr(image_recognition::take_screenshot());
	std::cout << "result: " << std::endl;
	for (auto iter = population.begin(); iter != population.end(); iter++) {
		std::cout << iter->first << ": " << iter->second << std::endl;
	}

	web::json::value json_message;
	for (const auto& p : population) {
		json_message[std::wstring(p.first.begin(), p.first.end())] = web::json::value::number(p.second);
	}
	json_message[U("version")] = web::json::value(std::wstring(VERSION_TAG.begin(), VERSION_TAG.end()));

	web::http::http_response response(status_codes::OK);
	response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
	response.set_body(json_message);
	const auto t = request.reply(response);
};