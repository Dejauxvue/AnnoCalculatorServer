#pragma once

#include <set>
#include <map>
#include <string>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace reader
{
class image_recognition;
}



struct wish
{
	std::string name;
	unsigned int guid;
	unsigned int count;
};

class item_wishlist
{
public:
	item_wishlist(reader::image_recognition& recog,
		const std::string& path);

	bool contains(unsigned int guid) const;

	bool buy_from(unsigned int trader_guid) const;

	std::string get_language() const;
	unsigned int get_max_reroll_costs() const;

	void bought(unsigned int guid);

private:
	std::string path;
	reader::image_recognition& recog;
	std::map<unsigned int, wish> items;
	std::map<unsigned int, std::set<unsigned int>> traders;

	// settings
	std::string language;
	bool delete_bought;
	unsigned int max_reroll_costs;

	void load();
	void save();
};