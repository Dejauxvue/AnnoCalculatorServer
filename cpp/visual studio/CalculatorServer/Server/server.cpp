#include "server.hpp"


#include "image_recognition.hpp"




using namespace std;
using namespace web;
using namespace utility;
using namespace http;
using namespace web::http::experimental::listener;

server::server(utility::string_t url) : m_listener(url)
{
	m_listener.support(methods::GET, std::bind(&server::handle_get, this, std::placeholders::_1));
}


void server::handle_get(http_request request)
{
	ucout << request.to_string() << endl;

	//auto population = image_recognition::get_anno_population_template_matching();
	auto population = image_recognition::get_anno_population_tesserarct_ocr();
	std::cout << "result: " << std::endl;
	for (auto iter= population.begin(); iter != population.end(); iter++) {
		std::cout << iter->first << ": " << iter->second << std::endl;
	}

	web::json::value json_message;
	for (const auto& p : population) {
		json_message[std::wstring(p.first.begin(), p.first.end())] = web::json::value::number(p.second);
	}

	web::http::http_response response(status_codes::OK);
	response.headers().add(U("Access-Control-Allow-Origin"), U("*"));
	response.set_body(json_message);
	request.reply(response);
};