#include "version.hpp"
#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

namespace reader
{
using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

/* IMPORTANT!!: this should be updated to the current release tag*/
const std::string version::VERSION_TAG = std::string("v10.1");

void version::check_and_log()
{
    // Create http_client to send the request.
    http_client client(U("https://api.github.com/"));

    // Build request URI and start the request.
    uri_builder builder(U("/repos/NiHoel/Anno1800UXEnhancer/releases/latest"));
    //builder.append_query(U("q"), U("cpprestsdk github"));
    client.request(methods::GET, builder.to_string())
        .then([=](http_response response)
            {
                if (response.status_code() == status_codes::OK)
                {
                    response.headers().set_content_type(L"application/json");

                    return response.extract_json();
                }

                return pplx::task_from_result(json::value());
            })
        .then([](pplx::task<json::value> previousTask)
                {
                    try
                    {
                        const json::value& v = previousTask.get();
                        if (v.has_string_field(U("tag_name")))
                        {
                            std::string latest_version(utility::conversions::to_utf8string(v.at(U("tag_name")).as_string()));
                            if (latest_version.compare(VERSION_TAG) == 0)
                            {
                                std::cout << "Version up to date." << std::endl;
                                return;
                            }
                            else
                            {
                                std::cout << "New version " << latest_version << " available at" << std::endl
                                    << "https://github.com/NiHoel/Anno1800UXEnhancer/releases/latest" << std::endl;
                                return;
                            }

                        }
                        
                    }
                    catch (const http_exception&)
                    {

                    }

                    std::cout << "Version check failed. New versions are available here:" << std::endl
                        << "https://github.com/NiHoel/Anno1800UXEnhancer/releases/latest" << std::endl;
                });
 }

}
