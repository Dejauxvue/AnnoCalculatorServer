#pragma once

#include <set>
#include <map>
#include <string>

#include <vcclr.h>


#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>


using namespace System;
using namespace System::Collections::Generic;


namespace reader
{
class image_recognition;
}

class item_wishlist
{
public:
	friend class configuration;

	static const unsigned int MAX_COUNT = 9999;

	item_wishlist(reader::image_recognition& recog);

	bool contains(unsigned int guid) const;

	bool buy_from(unsigned int trader_guid) const;

	std::map<unsigned int, unsigned int>::const_iterator begin() const;
	std::map<unsigned int, unsigned int>::const_iterator end() const;

	/**
	* @returns state changed
	*/
	bool bought(unsigned int guid);

	void set_count(unsigned int guid, unsigned int count);
	void set_count(Collections::Generic::IEnumerable<AssetViewer::Data::TemplateAsset^>^ list);

	void register_callback(std::function<void(unsigned int, unsigned int)>&& f);

	bool is_busy() const;

	gcroot<System::Object^> m_lock;

private:
	reader::image_recognition& recog;
	std::map<unsigned int, unsigned int> items;
	std::map<unsigned int, std::set<unsigned int>> traders;
	std::vector<std::function<void(unsigned int, unsigned int)>> callbacks;

	bool busy;

	void load(const boost::property_tree::ptree& tree);
	boost::property_tree::ptree serialize() const;

	void notify(unsigned int, unsigned int) const;
};

class configuration
{
public:
	item_wishlist wishlist;

	configuration(reader::image_recognition& recog, const std::string& path);

	std::string get_language() const;
	unsigned int get_max_reroll_costs() const;
	void set_max_reroll_costs(int costs);

	bool was_loaded_successfull() const;

	void set_language(const std::string& language);

private:
	std::string path;
	reader::image_recognition& recog;

	// settings
	std::string language;
	unsigned int max_reroll_costs;

	bool loaded_successful;

	/*
	@returns successful
	*/
	bool load();
	void save();
};

