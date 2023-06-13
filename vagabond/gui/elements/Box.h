// Copyright (C) 2021 Helen Ginn

#ifndef __practical__Box__
#define __practical__Box__

#include "SimplePolygon.h"
#include "DragResponder.h"
#include "ButtonResponder.h"
#include <vagabond/core/Responder.h>

class Slider;

class Box : public SimplePolygon, public HasResponder<Responder<Box>>
{
public:
	Box();
	
	virtual void addQuad(double z = 0);

	virtual void addSplitQuad(double z = 0);
	void rescale(double x, double y);
	virtual void makeQuad();
	
	virtual bool mouseOver();
	virtual void unMouseOver();
	
	void addAltTag(std::string text);
};

#endif
