#include "gui.hpp"





// helper method since "a local lambda is not allowed in a member function of a managed class"
static void registerItemCountChangedCallback(configuration& config, ItemCountChanged^ action)
{
	gcroot< ItemCountChanged^> wrappedAction(action);
	config.wishlist.register_callback([wrappedAction](unsigned int guid, unsigned int count) {
		wrappedAction->Invoke(guid, count);
		});
}

static Data::Languages getLanguage(const std::string& language)
{
	switch (language[0]) {
	case 'b':
		return Data::Languages::Brazilian;
		break;

	case 'c':
		return Data::Languages::Chinese;
		break;

	case 'e':
		return Data::Languages::English;
		break;

	case 'f':
		return Data::Languages::French;
		break;

	case 'g':
		return Data::Languages::German;
		break;

	case 'i':
		return Data::Languages::Italian;
		break;

	case 'j':
		return Data::Languages::Japanese;
		break;

	case 'k':
		return Data::Languages::Korean;
		break;

	case 'p':
		return Data::Languages::Polish;
		break;

	case 'r':
		return Data::Languages::Russian;
		break;

	case 's':
		return Data::Languages::Spanish;
		break;

	default:
		return Data::Languages::English;
	}
}


Gui::Gui(configuration& config)
	:
	config(&config)
{
	thread = gcnew Thread(gcnew ThreadStart(this, &Gui::run));
	thread->SetApartmentState(ApartmentState::STA);
	thread->Name = "GUI";
	thread->Start();
}

void Gui::join()
{
	thread->Join();
}

#include <iostream>
void Gui::run()
{
	app = gcnew App();
	app->InitializeComponent();

	AssetProvider::CountMode = true;
	if (config->was_loaded_successfull())
		AssetProvider::SetLanguage(getLanguage(config->get_language()));
	else
		onLanguageChanged();

	for (const auto& entry : config->wishlist)
		for each (AssetViewer::Data::TemplateAsset ^ item in AssetProvider::GetItemsById(entry.first))
			item->CountMode->Count = entry.second;

	AssetProvider::MaxRerollCosts = config->get_max_reroll_costs();

	AssetProvider::OnLanguage_Changed += gcnew System::Action(this, &Gui::onLanguageChanged);
	AssetProvider::OnAssetCountChanged::add(gcnew System::Action<System::Collections::Generic::IEnumerable<AssetViewer::Data::TemplateAsset^>^>(this, &Gui::onAssetCountChanged));
	AssetProvider::OnRerollCostsChanged::add(gcnew System::Action<int>(this, &Gui::onRerollCostsChanged));
	registerItemCountChangedCallback(*config, gcnew ItemCountChanged(this, &Gui::onItemCountChanged));

	app->Run();

};

void Gui::onAssetCountChanged(Collections::Generic::IEnumerable<AssetViewer::Data::TemplateAsset^>^ list)
{
	config->wishlist.set_count(list);
}


void Gui::onLanguageChanged()
{
	msclr::interop::marshal_context context;
	std::string language = context.marshal_as<std::string>(Enum::GetName(AssetProvider::Language.GetType(), AssetProvider::Language)->ToLower());

	config->set_language(language);
}

void Gui::onRerollCostsChanged(int rerollCosts)
{
	config->set_max_reroll_costs(rerollCosts);
}

void Gui::onItemCountChanged(unsigned int guid, unsigned int count)
{
	for each (AssetViewer::Data::TemplateAsset ^ item in AssetProvider::GetItemsById(guid))
	{
		item->CountMode->Count = count;
	}
}

