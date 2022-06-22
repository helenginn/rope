// Copyright (C) 2021 Helen Ginn

#ifndef __practical__DragResponder__
#define __practical__DragResponder__

#include <string>

class DragResponder
{
public:
	DragResponder() {};
	virtual ~DragResponder() {};
	virtual void finishedDragging(std::string tag, double x, double y) = 0;
};


#endif
