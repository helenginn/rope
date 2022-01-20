// Copyright (C) 2021 Helen Ginn

#ifndef __practical__Draggable__
#define __practical__Draggable__

#include "Image.h"

class DragResponder;

class Draggable : virtual public Box
{
public:
	Draggable(DragResponder *sender = NULL);
	
	void setSender(DragResponder *sender)
	{
		_sender = sender;
	}
	
	void setDragSteps(int xstep, int ystep)
	{
		_xstep = xstep;
		_ystep = ystep;
	}
	
	void setDragBox(double xspan, double yspan)
	{
		_xspan = xspan;
		_yspan = yspan;
	}

	void setReturnTag(std::string tag)
	{
		_tag = tag;
	}
	
	std::string tag()
	{
		return _tag;
	}
	
	virtual void unMouseOver();
	virtual bool mouseOver();

	void setProportion(double x, double y);
	virtual void drag(double x, double y);
	virtual void setPosition(glm::vec3 pos);
protected:
	DragResponder *_sender;
private:
	glm::vec3 _centre;
	std::string _tag;
	double _xspan;
	double _yspan;
	int _xstep;
	int _ystep;
};

class DraggableImage : public Image, public Draggable
{
public:
	DraggableImage(std::string filename, DragResponder *sender = NULL)
	: Image(filename),
	Draggable(sender)
	{}

};

#endif
