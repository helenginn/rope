// vagabond
// Copyright (C) 2022 Helen Ginn
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Please email: vagabond @ hginn.co.uk for more details.

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include <curl/curl.h>
#include <vagabond/utils/extra_curl_utils.h>
#endif

#define _USE_MATH_DEFINES
#include <vagabond/utils/version.h>
#include <math.h>
#include "AddEntity.h"
#include "ChooseEntity.h"
#include "EntitySequenceView.h"
#include "CalculateMetadata.h"

#include <vagabond/core/Environment.h>
#include <vagabond/core/EntityManager.h>
#include <vagabond/core/Sequence.h>
#include <vagabond/core/Chain.h>
#include <vagabond/core/Entity.h>

#include <vagabond/gui/DisplayOptions.h>
#include <vagabond/gui/FixIssuesView.h>
#include <vagabond/gui/ConfSpaceView.h>
#include <vagabond/gui/SearchPDB.h>
#include <vagabond/gui/SerialRefiner.h>
#include <vagabond/gui/PathFinding.h>
#include <vagabond/gui/PathsMenu.h>
#include <vagabond/gui/BFactors.h>

#include <vagabond/gui/elements/ImageButton.h>
#include <vagabond/gui/elements/AskYesNo.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/TextEntry.h>
#include <vagabond/gui/elements/TextButton.h>
#include <vagabond/gui/elements/Menu.h>

AddEntity::AddEntity(Scene *prev, Chain *chain) : Scene(prev), AddObject(prev)
{
	_chain = chain;
	_obj.setSequence(_chain->fullSequence());
}

AddEntity::AddEntity(Scene *prev, std::string str) : Scene(prev), AddObject(prev)
{
	Sequence *seq = new Sequence(str);
	_obj.setSequence(seq);
}

AddEntity::AddEntity(Scene *prev, PolymerEntity *ent) : 
Scene(prev), AddObject(prev, ent)
{

}

void AddEntity::showFirstPage()
{
	float top = 0.3;
	float inc = 0.08;
	top += inc;

	deleteTemps();

	if (!_existing && _obj.hasSequence())
	{
		{
			Text *t = new Text("Reference sequence:");
			t->setLeft(0.15, top);
			t->addAltTag("Models of this entity will align to this sequence");
			addObject(t);
		}
		{
			TextButton *t = SequenceView::button(_obj.sequence(), this);
			t->setReturnTag("sequence");
			t->setRight(0.85, top);
			addObject(t);
		}
	}
	else if (_obj.hasSequence())
	{
		ImageButton *button = new ImageButton("assets/images/sequence.png", this);
		button->resize(0.2);
		button->setReturnTag("sequence");
		button->setCentre(0.2, 0.3);
		addTempObject(button);

		Text *text = new Text("Sequence");
		text->resize(0.8);
		text->setCentre(0.2, 0.4);
		addTempObject(text);
	}
	
	top += inc;

	if (_existing)
	{
		{
			std::string str = i_to_str(_obj.modelCount()) + " models / ";
			str += i_to_str(_obj.moleculeCount()) + " molecules";
			Text *t = new Text(str);
			t->setCentre(0.5, 0.15);
			t->resize(0.8);
			addTempObject(t);
		}

		{
			ImageButton *button = new ImageButton("assets/images/confspace.png", 
			                                      this);
			button->resize(0.2);
			button->setReturnTag("conf_space");
			button->setCentre(0.4, 0.3);
			addTempObject(button);

			Text *text = new Text("RoPE space");
			text->setCentre(0.4, 0.4);
			text->resize(0.8);
			addTempObject(text);
		}

		top += inc;

		{
			ImageButton *button = new ImageButton("assets/images/us28.png", 
			                                      this);
			button->resize(0.12);
			button->setReturnTag("refine");
			button->setCentre(0.2, 0.6);
			addTempObject(button);

			Text *text = new Text("Refine all");
			text->setCentre(0.2, 0.7);
			text->resize(0.8);
			addTempObject(text);
		}

		{
			ImageButton *button = new ImageButton("assets/images/search.png", 
			                                      this);
			button->resize(0.2);
			button->setReturnTag("search_pdb");
			button->setCentre(0.6, 0.3);
			addTempObject(button);

			Text *text = new Text("Search PDB");
			text->setCentre(0.6, 0.4);
			text->resize(0.8);
			addTempObject(text);
		}

		{
			ImageButton *button = new ImageButton("assets/images/misc_data.png", 
			                                      this);
			button->resize(0.2);
			button->setReturnTag("metadata");
			button->setCentre(0.8, 0.3);
			addTempObject(button);

			Text *text = new Text("Add metadata");
			text->setCentre(0.8, 0.4);
			text->resize(0.8);
			addTempObject(text);
		}

		{
			ImageButton *button = new ImageButton("assets/images/eye.png", 
			                                      this);
			button->resize(0.2);
			button->setReturnTag("display_options");
			button->setCentre(0.4, 0.6);
			addTempObject(button);

			Text *text = new Text("Display options");
			text->setCentre(0.4, 0.7);
			text->resize(0.8);
			addTempObject(text);
		}

		if (_obj.hasSequence())
		{
			ImageButton *b = new ImageButton("assets/images/phenylalanine.png", 
			                                      this);
			b->resize(0.15);
			b->setReturnTag("fix_issues");
			b->setCentre(0.6, 0.6);
			addTempObject(b);

			Text *text = new Text("Fix issues");
			text->setCentre(0.6, 0.7);
			text->resize(0.8);
			addTempObject(text);
		}

	if (_obj.hasSequence())
	{
		ImageButton *b = new ImageButton("assets/images/map.png", this);
		b->resize(0.15);
		b->setReturnTag("path_finding");
		b->setCentre(0.79, 0.58);
		addTempObject(b);

		Text *text = new Text("Pathfinding");
		text->setCentre(0.79, 0.7);
		text->resize(0.8);
		addTempObject(text);
	}

#ifdef VERSION_EXTEND_ENTITY_MENU
		ImageButton *button = ImageButton::arrow(-90, this,
		                                         "assets/images/big_button.png");
		button->setCentre(0.92, 0.5);
		button->resize(3.f);
		button->setReturnTag("second_page");
		addTempObject(button);
#endif
	}
}

void AddEntity::showSecondPage()
{
	deleteTemps();

	{
		ImageButton *button = new ImageButton("assets/images/bee.png", 
		                                      this);
		button->resize(0.19);
		button->setReturnTag("b_factors");
		button->setCentre(0.2, 0.3);
		addTempObject(button);

		Text *text = new Text("B factors");
		text->setCentre(0.2, 0.4);
		text->resize(0.8);
		addTempObject(text);
	}

	if (_obj.hasSequence())
	{
		ImageButton *b = new ImageButton("assets/images/tube_1.5ml.png", this);
		b->resize(0.15);
		b->setReturnTag("refinement");
		b->setCentre(0.4, 0.3);
		addTempObject(b);

		Text *text = new Text("Refinement prep");
		text->setCentre(0.4, 0.4);
		text->resize(0.8);
		addTempObject(text);
	}

	ImageButton *button = ImageButton::arrow(90, this,
	                                         "assets/images/big_button.png");
	button->resize(3.f);
	button->setCentre(0.08, 0.5);
	button->setReturnTag("first_page");
	addTempObject(button);

}

void AddEntity::setup()
{
	if (!_existing)
	{
		addTitle("New entity");
	}
	else
	{
		addTitle("Entity - " + _obj.name());
	}

	float top = 0.3;
	float inc = 0.08;

	if (!_existing)
	{
		{
			Text *t = new Text("Entity name:");
			t->setLeft(0.15, top);
			t->addAltTag("Unique identifier for entity");
			addObject(t);
		}
		{
			std::string file = _obj.name();
			textOrChoose(file, "Enter...");

			TextEntry *t = new TextEntry(file, this);
			t->setReturnTag("enter_name");
			t->setRight(0.85, top);

			if (_existing) { t->setInert(); }

			_name = t;
			addObject(t);
		}
	}
	
	showFirstPage();

	AddObject::setup();
}

void AddEntity::textOrChoose(std::string &file, std::string other)
{
	if (file.length() == 0)
	{
		file = other;
	}
}

void AddEntity::refreshInfo()
{
	{
		std::string text = _obj.name();
		textOrChoose(text, "Enter...");
		_name->setText(text);
	}
}

void AddEntity::searchPdb()
{
	SearchPDB *search = new SearchPDB(this, &_obj);
	search->show();

}

template <class Func>
void runFunctionIfModelsExist(Entity *entity, AddEntity *add, const Func &func)
{
	try
	{
		if (entity->modelCount()==0)
		{
			throw std::invalid_argument("No models detected, have you modelled"\
			" your entities?");
		}

		func();
	}
	catch(std::exception &err)
	{
		BadChoice *brr = new BadChoice(add, err.what());
		brr->setDismissible(true);
		add->setModal(brr);
	}
}

void AddEntity::loadConfSpaceView(std::string suffix)
{
	ConfSpaceView *view = new ConfSpaceView(this, &_obj);
	if (suffix == "refined_torsions")
	{
		view->setMode(rope::ConfTorsions);
	}
	else if (suffix == "atom_positions")
	{
		view->setMode(rope::ConfPositional);
	}
	else if (suffix == "b_factor_space")
	{
		view->setMode(rope::ConfBFactor);
	}

	view->show();
}

void AddEntity::serialRefinement()
{
	SerialRefiner *refiner = new SerialRefiner(this, &_obj);
	refiner->setJobType(rope::ThoroughRefine);
	refiner->show();
}

void AddEntity::buttonPressed(std::string tag, Button *button)
{
	std::string suffix = Button::tagEnd(tag, "conf_space_");
	
	if (suffix.length() > 0)
	{
		auto load_confs = [this, suffix]()
		{
			loadConfSpaceView(suffix);
		};
		
		runFunctionIfModelsExist(&_obj, this, load_confs);
	}

	if (tag == "enter_name")
	{
		_obj.setName(_name->scratch());
		refreshInfo();
	}
	else if (tag == "second_page" && _existing)
	{
		showSecondPage();
	}
	else if (tag == "first_page" && _existing)
	{
		showFirstPage();
	}
	else if (tag == "sequence" && _existing)
	{
		EntitySequenceView *view = new EntitySequenceView(this, _obj.sequence());
		view->setEntity(&_obj);
		view->show();
	}
	else if (tag == "sequence" && !_existing)
	{
		SequenceView *view = new SequenceView(this, _obj.sequence());
		view->show();
	}
	else if (tag == "conf_space" && button->left())
	{
		auto load_confs = [this, tag]()
		{
			loadConfSpaceView(tag);
		};
		
		runFunctionIfModelsExist(&_obj, this, load_confs);
	}
	else if (tag == "conf_space" && !button->left())
	{
		Menu *m = new Menu(this, this, "conf_space");
		m->addOption("refined torsions", "refined_torsions");
		m->addOption("atom positions", "atom_positions");
		m->addOption("B factors", "b_factor_space");
		m->setup(button);
		setModal(m);
	}
	else if (tag == "b_factors")
	{
		BFactors *bf = new BFactors(this, &_obj);
		bf->show();
	}
	else if (tag == "search_pdb")
	{
		searchPdb();
	}
	else if (tag == "metadata")
	{
		CalculateMetadata *cm = new CalculateMetadata(this, &_obj);
		cm->show();
	}
	else if (tag == "refine")
	{
		auto refine_models = [this]()
		{
			serialRefinement();
		};

		runFunctionIfModelsExist(&_obj, this, refine_models);
	}
	else if (tag == "fix_issues")
	{
		FixIssuesView *fiv = new FixIssuesView(this, &_obj);
		fiv->show();
		return;
	}
	else if (tag == "display_options")
	{
		DisplayOptions *dov = new DisplayOptions(this, &_obj);
		dov->show();
		return;
	}
	else if (tag == "path_finding")
	{
		PathsMenu *pm = new PathsMenu(this, &_obj);
		pm->show();
		return;
	}
	else if (tag == "refinement")
	{
		PathFinding *pf = new PathFinding(this, &_obj); // for warp
		pf->show();
		return;
	}
	else if (tag == "create")
	{
		if (_obj.name() == "")
		{
			_obj.setName(_name->scratch());
		}

		try
		{
			Environment::entityManager()->insertIfUnique(_obj);
			back();
			if (_caller != nullptr)
			{
				_caller->setEntity(_obj.name());
			}
		}
		catch (const std::runtime_error &err)
		{
			BadChoice *bad = new BadChoice(this, err.what());
			setModal(bad);
		}
	}
	else if (tag == "delete" && _existing)
	{
		AskYesNo *ayn = new AskYesNo(this, "Are you sure you want to delete\n"\
		                             "the entity?");
		ayn->addJob("yes", [this]()
		            {
			           Environment::purgeEntity(_obj.name());
			           back();
			        });
			
		setModal(ayn);
	}
	else if (tag == "back" && _existing)
	{
		Environment::entityManager()->update(_obj);
	}

	AddObject::buttonPressed(tag, button);
}

