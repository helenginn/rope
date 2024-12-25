#include "HBondMenu.h"
#include "Display.h"
#include "DisplayUnit.h"
#include <vagabond/gui/elements/Text.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/core/Environment.h>

HBondMenu::HBondMenu(Scene *prev) : ListView(prev)
{
	_manager = Environment::hBondManager();
	// _manager->Manager::setResponder(this);
	_manager->HasResponder<Responder<HBondManager>>::setResponder(this);
}

HBondMenu::~HBondMenu()
{

}

void HBondMenu::setup()
{
	addTitle("H-Bond menu");
	ListView::setup();
}

void HBondMenu::refresh()
{
	ListView::refresh();

	if (_currHBond != nullptr)
	{
		_currHBond->unload();
		_currHBond = nullptr;
	}

}

size_t HBondMenu::lineCount()
{
	return _manager->objectCount();
}

Renderable *HBondMenu::getLine(int i)
{
	std::string filename = _manager->object(i).source();
	Box *b = new Box();
	
	TextButton *tbutton = new TextButton(_manager->object(i).source(), this);
	tbutton->setReturnTag("hbonds_" + filename);
	tbutton->setLeft(0.0, 0.0);
	b->addObject(tbutton);

	if (_selectMode)
	{
			TextButton *tselect = new TextButton("[select]", this);
			tselect->setReturnTag("select_hbonds_" + filename);
			tselect->setRight(0.6, 0.0);
			b->addObject(tselect);
		
	}

	return b;
}

void HBondMenu::buttonPressed(std::string tag, Button *button)
{

	ListView::buttonPressed(tag, button);
	std::string filename = Button::tagEnd(tag, "hbonds_");
	if (filename.length() >0)
	{	
		File *file = File::loadUnknown(filename);
		HBondDataView *view = new HBondDataView(this, file->hBondData());
		view->show();
	}

	if (tag.rfind("select_hbonds_", 0) == 0)
	{

		std::string filename = Button::tagEnd(tag, "select_hbonds_");

		if (!filename.empty())
		{	
			// std::vector<HBondManager::HBondPair> _hbondPairs = _manager->generateDonorAcceptorPairs(filename);
			auto hbondPairs = _manager->generateDonorAcceptorPairs(filename);
            if (_callback)
			{
				_callback(hbondPairs);
			}
            // }
		}
	}
	Scene::buttonPressed(tag, button);
}


void HBondMenu::respond()
{
	refreshNextRender();
}


































