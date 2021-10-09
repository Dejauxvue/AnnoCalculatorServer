#pragma once

#include <vcclr.h>
#include <msclr/marshal_cppstd.h>

#include "configuration.hpp"

using namespace System;
using namespace System::Threading;

using namespace AssetViewer;

delegate void AssetCountChanged(Collections::Generic::IEnumerable<AssetViewer::Data::TemplateAsset^>^);
delegate void ItemCountChanged(unsigned int, unsigned int);


static Data::Languages getLanguage(const std::string& language);


ref class Gui
{
public:
	Gui(configuration& config);
	
	void join();

private:
	App^ app;
	configuration* config;
	Thread^ thread;

	void run();

	void onAssetCountChanged(Collections::Generic::IEnumerable<AssetViewer::Data::TemplateAsset^>^ list);

	void onLanguageChanged();

	void onRerollCostsChanged(int rerollCosts);

	void onItemCountChanged(unsigned int guid, unsigned int count);
};

