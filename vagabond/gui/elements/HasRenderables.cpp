// Copyright (C) 2021 Helen Ginn

#include <algorithm>
#include <SDL2/SDL.h>
#include <cfloat>
#include <iostream>
#include "HasRenderables.h"
#include "Renderable.h"

void HasRenderables::deleteObjects(bool recursive)
{
	for (size_t i = 0; i < objectCount() && recursive; i++)
	{
		object(i)->deleteObjects();
	}

	for (int i = _objects.size() - 1; i >= 0; i--)
	{
		Renderable *r = _objects[i];
		if (_dragged == r)
		{
			_dragged = nullptr;
		}

		if (_chosen == r)
		{
			_chosen = nullptr;
		}

		_objects.erase(_objects.begin() + i);
		delete r;
	}

	clearObjects();
}

void HasRenderables::clearHighlights()
{
	for (size_t i = 0; i < objectCount(); i++)
	{
		object(i)->clearHighlights();
		object(i)->unMouseOver();
	}
	
}

Renderable *HasRenderables::findObject(double x, double y)
{
	double z = -FLT_MAX;
	Renderable *chosen = NULL;
	
	std::vector<Renderable *> objs = pertinentObjects();

	for (size_t i = 0; i < objs.size(); i++)
	{
		objs[i]->unMouseOver();
	}

	for (int i = objs.size() - 1; i >= 0; i--)
	{
		Renderable *r = objs[i];

		/* search inside the object first */
		chosen = r->findObject(x, y);
		
		if (chosen != NULL)
		{
			break;
		}
		
		if (!r->isSelectable() || r->isDisabled())
		{
			continue;
		}
		
		/* if not then search the object itself */
		bool hit = r->intersectsRay(x, y, &z);

		if (hit)
		{
			chosen = r;
			break;
		}
	}

	return chosen;
}

void HasRenderables::removeObject(Renderable *obj)
{
	if (obj == nullptr)
	{
		return;
	}

	if (_chosen == obj)
	{
		_chosen = nullptr;
	}

	if (_dragged == obj)
	{
		_dragged = nullptr;
	}

	std::vector<Renderable *>::iterator it;
	
	it = std::find(_objects.begin(), _objects.end(), obj);
	
	if (it != _objects.end())
	{
		_objects.erase(it);
	}
}

void HasRenderables::transferObjectToOther(Renderable *r, HasRenderables *h)
{
	removeObject(r);
	h->addObject(r);
}

HasRenderables::~HasRenderables()
{
	deleteObjects();
}

void HasRenderables::addObjectToFront(Renderable *r)
{
	std::vector<Renderable *>::iterator it;
	
	it = std::find(_objects.begin(), _objects.end(), r);
	
	if (it == _objects.end())
	{
		_objects.insert(_objects.begin(), r);
	}
	else
	{
		throw(std::runtime_error("trying to add same object twice"));
	}

}

void HasRenderables::addObject(Renderable *r)
{
	if (r == nullptr)
	{
		return;
	}

	std::vector<Renderable *>::iterator it;
	
	it = std::find(_objects.begin(), _objects.end(), r);
	
	if (it == _objects.end())
	{
		modify(r);
		_objects.push_back(r);
	}
	else
	{
		throw(std::runtime_error("trying to add same object twice"));
	}
}

void HasRenderables::addTempObject(Renderable *r)
{
	addObject(r);
	_temps.push_back(r);
}

void HasRenderables::deleteTemps()
{
	for (Renderable *r : _temps)
	{
		if (_dragged == r)
		{
			_dragged = nullptr;
		}

		if (_chosen == r)
		{
			_chosen = nullptr;
		}

		removeObject(r);
		r->deleteObjects(true);
		delete r;
	}

	_temps.clear();
}

void HasRenderables::doThingsCircuit()
{
	for (size_t i = 0; i < objectCount(); i++)
	{
		object(i)->doThingsCircuit();
	}

	doThings();
}

Renderable *HasRenderables::tab(bool shift)
{
	if (_chosen)
	{
		_chosen->unMouseOver();
	}
	
	int idx = -1;
	
	if (_objects.size() == 0)
	{
		return nullptr;
	}
	
	/* find _chosen in our array */
	for (size_t i = 0; i < _objects.size(); i++)
	{
		if (_objects[i] == _chosen)
		{
			idx = i;
			break;
		}
	}

	int i = idx;
	shift ? i-- : i++;
	for (; i != idx; shift ? i-- : i++)
	{
		std::cout << "Next: " << i << " - ";
		if (i >= (int)_objects.size())
		{
			i = 0;
		}

		if (i < 0)
		{
			i = _objects.size() - 1;
		}
		
		std::cout << " now " << i << " - ";

		Renderable *r = _objects[i];

		if (r && (!r->isSelectable() || r->isDisabled()))
		{
			if (!r->isSelectable() && !r->isDisabled())
			{
				std::cout << "Checking inside " << r->name() << std::endl;
				Renderable *result = r->tab(shift);

				if (result)
				{
					_chosen = r;
					return r;
				}
			}

			std::cout << "Skipping " << r->name() << std::endl;
			continue;
		}

		std::cout << "Choosing " << r->name() << std::endl;
		_chosen = r;
		break;
	}

	if (_chosen == nullptr)
	{
		return nullptr;
	}
	
	if (_chosen->isSelectable())
	{
		_chosen->mouseOver();
	}

	return _chosen;
}

bool HasRenderables::enter(bool shift)
{
	if (_chosen)
	{
		bool shouldClick = _chosen->enter(shift);
		
		if (shouldClick)
		{
			_chosen->click();
		}
	}
	
	if (!_chosen)
	{
		return true;
	}

	return false;
}

void HasRenderables::doAccessibilityThings(SDL_Keycode pressed, bool shift)
{
	if (pressed == SDLK_TAB)
	{
		tab(shift);
	}
	else if (pressed == SDLK_RETURN)
	{
		enter(shift);
	}

}

