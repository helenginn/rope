// Copyright (C) 2021 Helen Ginn

#ifndef __practical__Slider__
#define __practical__Slider__

#include "Image.h"
#include "DragResponder.h"

class Text;

class Slider : public Image, public DragResponder
{
public:
	Slider();

	void setup(double min, double max, double step);

	virtual void finishedDragging(std::string tag, int x, int y);
	
	void setPrefixSuffix(std::string prefix, std::string suffix)
	{
		_prefix = prefix;
		_suffix = suffix;
	}
	
	double getValue()
	{
		return _val;
	}
private:
	double _min;
	double _max;
	double _step;

	double _val;
	Text *_display;

	std::string _prefix;
	std::string _suffix;
};

#endif
