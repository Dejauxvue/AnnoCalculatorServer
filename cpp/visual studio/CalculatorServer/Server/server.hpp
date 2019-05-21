#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>
#include <random>

#include "cpprest/json.h"
#include "cpprest/http_listener.h"
#include "cpprest/uri.h"
#include "cpprest/asyncrt_utils.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#else
# include <sys/time.h>
#endif

#include "server.hpp"

using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

class server
{
public:
	server() {}
	server(utility::string_t url);

	pplx::task<void> open() { return m_listener.open(); }
	pplx::task<void> close() { return m_listener.close(); }

private:
	web::json::value read_anno_population()const;

	void handle_get(http_request message);

	http_listener m_listener;
};
hi