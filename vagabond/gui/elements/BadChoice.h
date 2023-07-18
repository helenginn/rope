// Copyright (C) 2021 Helen Ginn

#ifndef __practical__BadChoice__
#define __practical__BadChoice__

#include "Modal.h"
#include "ButtonResponder.h"

class Scene;

/** \class BadChoice
 * (suggest class name change: Alert)
 * Provides an alert for the user - regardless of whether it was a bad
 * choice or not. Can be dismissed by clicking outside the box. */

class BadChoice : public Modal
{
public:
/** @param scene which should display this modal alert
 *  @param details message to display, pre-formatted with line breaks. */
	BadChoice(Scene *scene, std::string details);

	virtual void buttonPressed(std::string tag, Button *button = NULL);
private:

};

#endif
