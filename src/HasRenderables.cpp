// Copyright (C) 2021 Helen Ginn

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
		bool hit = r->intersectsPolygon(x, y, &z);

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
