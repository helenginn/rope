// Copyright (C) 2021 Helen Ginn

#ifndef __practical__HasRenderables__
#define __practical__HasRenderables__

#include <vector>

class Renderable;

class HasRenderables
{
public:
	virtual ~HasRenderables();

	void clearObjects()
	{
		_objects.clear();
	}
	
	void deleteObjects();
	
	void addObject(Renderable *r);
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

	Renderable *findObject(double x, double y);
protected:
	void clearHighlights();
	std::vector<Renderable *> _objects;
	std::vector<Renderable *> _temps;

};

#endif

