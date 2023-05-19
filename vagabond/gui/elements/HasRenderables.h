// Copyright (C) 2021 Helen Ginn

#ifndef __practical__HasRenderables__
#define __practical__HasRenderables__

#include <vector>
#include <SDL2/SDL.h>

class Renderable;

class HasRenderables
{
public:
	virtual ~HasRenderables();

	void clearObjects()
	{
		_objects.clear();
	}
	
	void deleteObjects(bool recursive = true);
	
	void addObject(Renderable *r);
	bool hasObject(Renderable *r);
	virtual void modify(Renderable *r) {};
	void addTempObject(Renderable *r);
	
	void deleteTemps();

	void addObjectToFront(Renderable *r);

	size_t objectCount()
	{
		return _objects.size();
	}
	
	Renderable *object(int i)
	{
		return _objects[i];
	}
	
	std::vector<Renderable *> &objects()
	{
		return _objects;
	}

	virtual std::vector<Renderable *> &pertinentObjects()
	{
		return _objects;
	}

	void removeObject(Renderable *obj);
	
	void transferObjectToOther(Renderable *r, HasRenderables *h);

	virtual void doThingsCircuit();
	virtual void doThings() {};

	virtual Renderable *findObject(double x, double y);
	void doAccessibilityThings(SDL_Keycode pressed, bool shift);
protected:
	void clearHighlights();

	/** accessibility */
	Renderable *tab(bool shift);
	bool enter(bool shift);

	std::vector<Renderable *> _objects;
	std::vector<Renderable *> _temps;

	Renderable *_chosen = nullptr;
	Renderable *_dragged = nullptr;
};

#endif

