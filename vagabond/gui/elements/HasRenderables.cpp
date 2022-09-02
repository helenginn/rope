// Copyright (C) 2021 Helen Ginn

#include <algorithm>
#include <SDL2/SDL.h>
#include <cfloat>
#include <iostream>
#include "HasRenderables.h"
#include "Renderable.h"

void HasRenderables::deleteObjects()
{
	for (size_t i = 0; i < _objects.size(); i++)
	{
		delete _objects[i];
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

	for (size_t i = 0; i < objs.size(); i++)
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
	std::vector<Renderable *>::iterator it;
	
	it = std::find(_objects.begin(), _objects.end(), r);
	
	if (it == _objects.end())
	{
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
		removeObject(r);
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

void HasRenderables::tab(bool shift)
{
	std::vector<Renderable *>::iterator pos;
	std::vector<Renderable *>::iterator it;
	
	if (_chosen)
	{
		_chosen->unMouseOver();
	}
	
	/* find _chosen in our array and loop round until we find the next
	 * selectable object. */
	pos = std::find(_objects.begin(), _objects.end(), _chosen);

	if (pos == _objects.end())
	{
		for (Renderable *r : _objects)
		{
			if (!r->isSelectable() || r->isDisabled())
			{
				continue;
			}

			_chosen = r;
			break;
		}
	}
	else
	{
		it = pos;
		it++;
		for (; it != pos; (shift ? it-- : it++))
		{
			if (it == _objects.end())
			{
				it = _objects.begin();
			}

			Renderable *r = *it;

			if (!shift && it == _objects.begin())
			{
				it = _objects.end();
			}

			if (r && (!r->isSelectable() || r->isDisabled()))
			{
				continue;
			}

			_chosen = r;
			break;
		}
	}

	if (_chosen == nullptr)
	{
		return;
	}
	
	_chosen->mouseOver();
}

void HasRenderables::enter(bool shift)
{
	if (_chosen)
	{
		_chosen->click();
	}
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

