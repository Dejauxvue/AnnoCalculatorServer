#include "wishlist.hpp"

#include "reader_util.hpp"

item_wishlist::item_wishlist(reader::image_recognition& recog, const std::string& path)
	:
	recog(recog),
	path(path),
	deleteBought(false)
{
	load();
}

bool item_wishlist::contains(unsigned int guid) const
{
	auto iter = items.find(guid);
	return iter != items.end() && iter->second.count;
}

bool item_wishlist::buy_from(unsigned int trader_guid) const
{
	return traders.find(trader_guid) != traders.end();
}

std::string item_wishlist::get_language() const
{
	return language;
}

void item_wishlist::bought(unsigned int guid)
{
	auto iter = items.find(guid);
	if (iter == items.end())
		return;

	auto& count = iter->second.count;

	if(count && count != std::numeric_limits<unsigned int>::max())
		count--;

	bool erased = false;
	if (deleteBought && !count)
	{
		iter = items.erase(iter);
		erased = true;
	}
		
	save();

	if (!erased)
		return;

	for (unsigned int trader : recog.items[guid]->traders)
	{
		auto t_iter = traders.find(trader);
		if (t_iter == traders.end())
			continue;

		t_iter->second.erase(guid);
		if (t_iter->second.empty())
			traders.erase(t_iter);
	}
}

void item_wishlist::load()
{
	boost::property_tree::ptree pt;
	boost::property_tree::read_json(path, pt);

	auto st = pt.get_child_optional("settings");
	language = "english";
	if (st.has_value())
	{
		try {
			language = st.get().get_child("language").get_value<std::string>();
			if (!recog.has_language(language))
				throw std::invalid_argument(language);
		}
		catch (const std::exception & e)
		{
			language = "english";
		}

		try{
			deleteBought = st.get().get_child("deleteBought").get_value<bool>();
		}
		catch (const std::exception & e)
		{	}
	}

	recog.update(language);

	for (const auto& entry : pt.get_child("items"))
	{
		wish w;

		w.guid = entry.second.get_child("guid").get_value<unsigned int>();
		auto iter = recog.items.find(w.guid);
		if (iter == recog.items.end())
			throw std::invalid_argument(std::string("GUID: ") + std::to_string(w.guid) + " is not an item\nPlease check your configuration file!");
		


		try
		{
			w.count = entry.second.get_child("count").get_value<unsigned int>();
		}
		catch (const std::exception & e)
		{
			w.count = std::numeric_limits<unsigned int>::max();
		}

		if(w.count || !deleteBought)
			for (unsigned int trader : iter->second->traders)
			{
				auto t_iter = traders.find(trader);
				if (t_iter == traders.end())
				{
					traders.emplace(trader, std::set<unsigned int>({ w.guid }));
				}
				else
				{
					t_iter->second.emplace(w.guid);
				}
			}


		try
		{
			w.name = entry.second.get_child("name").get_value<std::string>();
		}
		catch (const std::exception & e)
		{
			w.name = recog.get_dictionary().items.find(w.guid)->second;
		}

		items.emplace(w.guid, w);
	}
}

void item_wishlist::save()
{
	boost::property_tree::ptree pt;
	boost::property_tree::ptree pt_val;
	boost::property_tree::ptree pt_settings;
	boost::property_tree::ptree pt_items;
	
	pt_val.put_value(language);
	pt_settings.push_back(std::make_pair("language", pt_val));
	pt_val.put_value(deleteBought);
	pt_settings.push_back(std::make_pair("deleteBought", pt_val));
	pt.push_back(std::make_pair("settings", pt_settings));

	for (const auto& entry : items)
	{
		const auto& item = entry.second;
		boost::property_tree::ptree pt_item;
		
		pt_val.put_value(item.name);
		pt_item.push_back(std::make_pair("name", pt_val));
		pt_val.put_value(item.guid);
		pt_item.push_back(std::make_pair("guid", pt_val));

		if (item.count != std::numeric_limits<unsigned int>::max())
		{
			pt_val.put_value(item.count);
			pt_item.push_back(std::make_pair("count", pt_val));
		}

		pt_items.push_back(std::make_pair("", pt_item));
	}

	pt.push_back(std::make_pair("items", pt_items));
	boost::property_tree::write_json(path,pt);
}
