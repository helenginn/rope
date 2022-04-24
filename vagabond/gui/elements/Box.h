// Copyright (C) 2021 Helen Ginn

#ifndef __practical__Box__
#define __practical__Box__

#include "Renderable.h"

class Box : public Renderable
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
protected:

};

#endif
