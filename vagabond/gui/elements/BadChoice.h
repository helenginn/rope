// Copyright (C) 2021 Helen Ginn

#ifndef __practical__BadChoice__
#define __practical__BadChoice__

#include "Modal.h"
#include "ButtonResponder.h"

class Scene;

class BadChoice : public Modal
{
public:
	BadChoice(Scene *scene, std::string details);

	virtual void buttonPressed(std::string tag, Button *button = NULL);
private:

};

#endif
