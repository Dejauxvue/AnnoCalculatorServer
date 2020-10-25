#include "configuration.hpp"

#include <msclr\lock.h>

#include "../CalculatorServer/reader/reader_util.hpp"

item_wishlist::item_wishlist(reader::image_recognition& recog)
	:
	recog(recog),
	m_lock(gcnew System::Object),
	busy(false)
{
}

bool item_wishlist::contains(unsigned int guid) const
{
	msclr::lock l(m_lock);
	auto iter = items.find(guid);
	return iter != items.end() && iter->second;
}

bool item_wishlist::buy_from(unsigned int trader_guid) const
{
	msclr::lock l(m_lock);
	return traders.find(trader_guid) != traders.end();
}

std::map<unsigned int, unsigned int>::const_iterator item_wishlist::begin() const
{
	return items.cbegin();
}

std::map<unsigned int, unsigned int>::const_iterator item_wishlist::end() const
{
	return items.cend();
}

bool item_wishlist::bought(unsigned int guid)
{
	msclr::lock l(m_lock);

	auto iter = items.find(guid);
	if (iter == items.end())
		return false;

	unsigned int count = iter->second;

	if(count && count != MAX_COUNT)
		set_count(guid, --count);

	

	return true;
}

void item_wishlist::set_count(unsigned int guid, unsigned int count)
{
	//std::cout << guid << ": " << count << std::endl;
	bool doNotify = false;
	{
		msclr::lock l(m_lock);

		auto recog_iter = recog.items.find(guid);
		if (recog_iter == recog.items.end())
			return;


		if (count && contains(guid))
		{
			if (items[guid] != count)
			{
				items[guid] = count;
				doNotify = true;
			}
		}
		else if (count && !contains(guid))
		{

			for (unsigned int trader : recog_iter->second->traders)
			{
				auto t_iter = traders.find(trader);
				if (t_iter == traders.end())
				{
					traders.emplace(trader, std::set<unsigned int>({ guid }));
				}
				else
				{
					t_iter->second.emplace(guid);
				}
			}

			items.emplace(guid, count);

			doNotify = true;
		}
		else if (!count)
		{
			auto items_iter = items.find(guid);

			if (items_iter != items.end())
			{
				items.erase(items_iter);
				for (unsigned int trader : recog.items[guid]->traders)
				{
					auto t_iter = traders.find(trader);
					if (t_iter == traders.end())
						continue;

					t_iter->second.erase(guid);
					if (t_iter->second.empty())
						traders.erase(t_iter);
				}

				doNotify = true;
			}
		}

	}

	if (doNotify)
		notify(guid, count);
}

void item_wishlist::set_count(Collections::Generic::IEnumerable<AssetViewer::Data::TemplateAsset^>^ list)
{
	busy = true;

	for each (AssetViewer::Data::TemplateAsset ^ item in list)
	{
		if (item->SetParts && item->SetParts->Count > 0)
			{
				for each (String ^ subitem in item->SetParts)
				{
					try {
						set_count(Int32::Parse(subitem), item->Count);
					}
					catch (const std::exception&)
					{
					}
				}
			} else
			{
				try
				{
				set_count(item->ID, item->Count);
				}
				catch (const std::exception&)
				{
				}
			}
	}

	busy = false;
	notify(0, 0);
}

void item_wishlist::register_callback(std::function<void(unsigned int, unsigned int)>&& f)
{
	callbacks.emplace_back(f);
}

bool item_wishlist::is_busy() const
{
	return busy;
}



void item_wishlist::load(const boost::property_tree::ptree& tree)
{
	msclr::lock l(m_lock);

	for (const auto& entry : tree)
	{
		unsigned int guid = entry.second.get_child("guid").get_value<unsigned int>();
		auto iter = recog.items.find(guid);
		if (iter == recog.items.end())
			continue;
		
		unsigned int count = 0;

		try
		{
			count = entry.second.get_child("count").get_value<unsigned int>();
			if (count > MAX_COUNT)
				count = MAX_COUNT;
		}
		catch (const std::exception &)
		{
			count = MAX_COUNT;
		}

		set_count(guid, count);

	}
}

boost::property_tree::ptree item_wishlist::serialize() const
{
	msclr::lock l(m_lock);

	boost::property_tree::ptree pt_val;
	boost::property_tree::ptree pt_items;

	for (const auto& entry : items)
	{
		unsigned int guid = entry.first;
		unsigned int count = entry.second;

		boost::property_tree::ptree pt_item;

		pt_val.put_value(recog.get_dictionary().items.find(guid)->second);
		pt_item.push_back(std::make_pair("name", pt_val));
		pt_val.put_value(guid);
		pt_item.push_back(std::make_pair("guid", pt_val));

		if (count != MAX_COUNT)
		{
			pt_val.put_value(count);
			pt_item.push_back(std::make_pair("count", pt_val));
		}

		pt_items.push_back(std::make_pair("", pt_item));
	}

	return pt_items;
}

void item_wishlist::notify(unsigned int guid, unsigned int count) const
{
	for (const auto& f : callbacks)
		f(guid, count);
}



configuration::configuration(reader::image_recognition& recog, const std::string& path)
	:
	wishlist(recog),
	path(path),
	recog(recog),
	language("english")
{
	loaded_successful = load();
	wishlist.register_callback([this](unsigned int guid, unsigned int count) {
		if(!wishlist.is_busy())
			save();
		});
}

unsigned int configuration::get_max_reroll_costs() const
{
	return max_reroll_costs;
}

bool configuration::was_loaded_successfull() const
{
	return loaded_successful;
}

void configuration::set_language(const std::string& language)
{
	if (recog.has_language(language))
	{
		this->language = language;
		save();
	}
}

std::string configuration::get_language() const
{
	return language;
}

bool configuration::load()
{
	
	boost::property_tree::ptree pt;
	try {
		boost::property_tree::read_json(path, pt);
	}
	catch (const std::exception&)
	{
		// configuration file not avialable or invalid,
		// load default config
		return false;
	}

	auto st = pt.get_child_optional("settings");
	language = "english";
	if (st.has_value())
	{
		try {
			language = st.get().get_child("language").get_value<std::string>();
			if (!recog.has_language(language))
				throw std::invalid_argument(language);
		}
		catch (const std::exception&)
		{
			language = "english";
		}

		try {
			max_reroll_costs = st.get().get_child("maxRerollCosts").get_value<unsigned int>();
		}
		catch (const std::exception&)
		{
		}
	}

	recog.update(language);

	if(pt.get_child_optional("items").has_value())
		wishlist.load(pt.get_child("items"));

	return true;
}

void configuration::save()
{
	boost::property_tree::ptree pt;
	boost::property_tree::ptree pt_val;
	boost::property_tree::ptree pt_settings;


	pt_val.put_value(language);
	pt_settings.push_back(std::make_pair("language", pt_val));
	if (max_reroll_costs)
	{
		pt_val.put_value(max_reroll_costs);
		pt_settings.push_back(std::make_pair("maxRerollCosts", pt_val));
	}
	pt.push_back(std::make_pair("settings", pt_settings));



	pt.push_back(std::make_pair("items", wishlist.serialize()));
	boost::property_tree::write_json(path, pt);
}



