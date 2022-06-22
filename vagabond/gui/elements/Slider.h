// Copyright (C) 2021 Helen Ginn

#ifndef __practical__Slider__
#define __practical__Slider__

#include "Image.h"
#include "DragResponder.h"

class DraggableImage;
class Text;
class Dot;

class Slider : public Image, public DragResponder
{
public:
	Slider();
	
	void setDragResponder(DragResponder *next)
	{
		_responder = next;
	}

	void setup(std::string text, double min, double max, double step);

	virtual void finishedDragging(std::string tag, double x, double y);
	
	void setPrefixSuffix(std::string prefix, std::string suffix)
	{
		_prefix = prefix;
		_suffix = suffix;
	}
	
	double value()
	{
		return _val;
	}

	virtual void setCentre(double x, double y);

	void setStart(double x, double y);
private:
	double _min;
	double _max;
	double _step;
	double _width;
	double _numsteps;

	double _val;
	Text *_display = nullptr;

	std::string _prefix;
	std::string _suffix;
	
	DraggableImage *_dot = nullptr;
	DragResponder *_responder = nullptr;
};

#endif
