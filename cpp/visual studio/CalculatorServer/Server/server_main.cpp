
#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#pragma comment(lib, "WS2_32.lib")

#include <string>

#include <iostream>


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
using namespace experimental::listener;

std::unique_ptr<server> g_http;

void on_initialize(bool verbose, std::wstring window_regex, const string_t& address)
{
	// Build our listener's URI from the configured address and the hard-coded path "MyServer/Action"

	uri_builder uri(address);
	uri.append_path(U("AnnoServer/Population"));

	auto addr = uri.to_uri().to_string();
	g_http = std::make_unique<server>(verbose, window_regex, addr);
	g_http->open().wait();

	ucout << utility::string_t(U("Listening for requests at: ")) << addr << std::endl;
}

void on_shutdown()
{
	g_http->close().wait();
}

int wmain(int argc, wchar_t* argv[])
{
	string_t port = U("8000");
	bool verbose = false;
	std::wstring window_regex;

	int i = 1;
	while (i < argc)
	{
		if (std::wcscmp(argv[i], U("-v")) == 0)
		{
			verbose = true;
			i++;
		}
		else if (std::wcscmp(argv[i], U("-w")) == 0)
		{
			window_regex = argv[i + 1];
			i += 2;
		} else if(std::wcscmp(argv[i], U("-p")) == 0)
		{
			port = argv[i + 1];
		}
		else
			i++;
	}

	utility::string_t address = U("http://localhost:");
	address.append(port);

	try {
		on_initialize(verbose, window_regex, address);
		std::cout << "Press ENTER to exit." << std::endl;

		std::string line;
		std::getline(std::cin, line);

		on_shutdown();
	}
	catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
	}
	return 0;
}